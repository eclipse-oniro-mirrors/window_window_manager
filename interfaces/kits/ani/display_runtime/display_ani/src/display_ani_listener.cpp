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

#include "display_ani_listener.h"
#include "window_manager_hilog.h"
#include "display_ani_utils.h"

namespace OHOS {
namespace Rosen {

DisplayAniListener::~DisplayAniListener()
{
    TLOGI(WmsLogTag::DMS, "[ANI]~AniDisplayListener");
}

void DisplayAniListener::SetMainEventHandler()
{
    auto mainRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (mainRunner == nullptr) {
        return;
    }
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainRunner);
}

void DisplayAniListener::AddCallback(const std::string& type, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] AddCallback is called, type = %{public}s", type.c_str());
    if (env_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(aniCallbackMtx_);
    aniCallback_[type].emplace_back(callback);
    TLOGI(WmsLogTag::DMS, "[ANI] AddCallback success aniCallback_ size: %{public}u!",
        static_cast<uint32_t>(aniCallback_[type].size()));
}

void DisplayAniListener::RemoveCallback(ani_env* env, const std::string& type, ani_ref callback)
{
    std::lock_guard<std::mutex> lock(aniCallbackMtx_);
    auto it = aniCallback_.find(type);
    if (it == aniCallback_.end()) {
        TLOGE(WmsLogTag::DMS, "[ANI] Listener no callback to remove");
        return;
    }
    auto& listeners = it->second;
    for (auto iter = listeners.begin(); iter != listeners.end();) {
        ani_boolean isEquals = 0;
        env->Reference_StrictEquals(callback, *iter, &isEquals);
        if (isEquals) {
            listeners.erase(iter);
        } else {
            iter++;
        }
    }
    TLOGD(WmsLogTag::DMS, "[ANI] Success remove callback, jsCallBack_ size: %{public}u!",
        static_cast<uint32_t>(listeners.size()));
}

void DisplayAniListener::RemoveAllCallback()
{
    std::lock_guard<std::mutex> lock(aniCallbackMtx_);
    for (auto& [typeString, callbacks] : aniCallback_) {
        for (auto callback : callbacks) {
            if (env_) {
                env_->GlobalReference_Delete(callback);
            }
        }
    }
    aniCallback_.clear();
}

void DisplayAniListener::OnCreate(DisplayId id)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin, displayId: %{public}" PRIu64, id);
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] this listener is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(aniCallbackMtx_);
    if (aniCallback_.empty()) {
        TLOGE(WmsLogTag::DMS, "[ANI] Ani callback is empty!");
        return;
    }
    auto it = aniCallback_.find(EVENT_ADD);
    if (it == aniCallback_.end()) {
        TLOGE(WmsLogTag::DMS, "[ANI] Add callback has not been registered");
        return;
    }
    std::vector<ani_ref> vec = it->second;
    TLOGI(WmsLogTag::DMS, "vec_callback size: %{public}d", vec.size());
    // find callbacks in vector
    for (ani_ref oneAniCallback : vec) {
        if (env_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "[ANI] null env");
            return;
        }
        ani_boolean undefRes = 0;
        env_->Reference_IsUndefined(oneAniCallback, &undefRes);
        if (undefRes) {
            TLOGE(WmsLogTag::DMS, "[ANI] oneAniCallback is undef");
            continue;
        }
        auto task = [env = env_, oneAniCallback, id] {
            DisplayAniUtils::CallAniFunctionVoid(env, "L@ohos/display/display;", "displayEventCallBack",
                nullptr, oneAniCallback, static_cast<ani_double>(id));
        };
        if (!eventHandler_) {
            TLOGE(WmsLogTag::DMS, "[ANI] get main event handler failed!");
            return;
        }
        eventHandler_->PostTask(std::move(task), "dms:AniDisplayListener::CreateCallBack", 0,
            AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
}

void DisplayAniListener::OnDestroy(DisplayId id)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin, displayId: %{public}" PRIu64, id);
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] this listener is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(aniCallbackMtx_);
    if (aniCallback_.empty()) {
        TLOGE(WmsLogTag::DMS, "[ANI] Ani callback is empty!");
        return;
    }
    auto it = aniCallback_.find(EVENT_REMOVE);
    if (it == aniCallback_.end()) {
        TLOGE(WmsLogTag::DMS, "[ANI] Remove callback has not been registered");
        return;
    }
    std::vector<ani_ref> vec = it->second;
    TLOGI(WmsLogTag::DMS, "vec_callback size: %{public}d", vec.size());
    // find callbacks in vector
    for (ani_ref oneAniCallback : vec) {
        if (env_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "[ANI] null env");
            return;
        }
        ani_boolean undefRes = 0;
        env_->Reference_IsUndefined(oneAniCallback, &undefRes);
        if (undefRes) {
            TLOGE(WmsLogTag::DMS, "[ANI] oneAniCallback is undef");
            continue;
        }
        auto task = [env = env_, oneAniCallback, id] {
            DisplayAniUtils::CallAniFunctionVoid(env, "L@ohos/display/display;", "displayEventCallBack",
                nullptr, oneAniCallback, static_cast<ani_double>(id));
        };
        if (!eventHandler_) {
            TLOGE(WmsLogTag::DMS, "[ANI] get main event handler failed!");
            return;
        }
        eventHandler_->PostTask(std::move(task), "dms:AniDisplayListener::CreateCallBack", 0,
            AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
}
void DisplayAniListener::OnChange(DisplayId id)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin, displayId: %{public}" PRIu64, id);
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] this listener is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(aniCallbackMtx_);
    if (aniCallback_.empty()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnChange not register!");
        return;
    }
    auto it = aniCallback_.find(EVENT_CHANGE);
    if (it == aniCallback_.end()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnChange not this event, return");
        return;
    }
    std::vector<ani_ref> vec = it->second;
    for (auto oneAniCallback : vec) {
        if (env_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "[ANI] null env");
            return;
        }
        ani_boolean undefRes = 0;
        env_->Reference_IsUndefined(oneAniCallback, &undefRes);
        if (undefRes) {
            TLOGE(WmsLogTag::DMS, "[ANI] oneAniCallback undef, return");
            continue;
        }
        auto task = [env = env_, oneAniCallback, id] {
            DisplayAniUtils::CallAniFunctionVoid(env, "L@ohos/display/display;", "displayEventCallBack",
                nullptr, oneAniCallback, static_cast<ani_double>(id));
        };
        if (!eventHandler_) {
            TLOGE(WmsLogTag::DMS, "[ANI] get main event handler failed!");
            return;
        }
        eventHandler_->PostTask(std::move(task), "dms:AniWindowListener::SizeChangeCallBack", 0,
            AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
}
void DisplayAniListener::OnPrivateWindow(bool hasPrivate)
{
    TLOGI(WmsLogTag::DMS, "[ANI] called, hasPrivate: %{public}u",
        static_cast<uint32_t>(hasPrivate));
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr || env_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] this listener or env is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(aniCallbackMtx_);
    if (aniCallback_.empty()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnPrivateWindow not register!");
        return;
    }
    if (aniCallback_.find(EVENT_PRIVATE_MODE_CHANGE) == aniCallback_.end()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnPrivateWindow not this event, return");
        return;
    }
    if (env_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }

    auto it = aniCallback_.find(EVENT_PRIVATE_MODE_CHANGE);
    for (auto oneAniCallback : it->second) {
        auto task = [env = env_, oneAniCallback, hasPrivate] () {
            DisplayAniUtils::CallAniFunctionVoid(env, "L@ohos/display/display;", "captureStatusChangedCallback",
                nullptr, oneAniCallback, hasPrivate);
        };
        if (!eventHandler_) {
            TLOGE(WmsLogTag::DMS, "[ANI] get main event handler failed!");
            return;
        }
        eventHandler_->PostTask(task, "dms:AniDisplayListener::PrivateWindowCallback", 0,
            AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
}
void DisplayAniListener::OnFoldStatusChanged(FoldStatus foldStatus)
{
    TLOGI(WmsLogTag::DMS, "[ANI] called, foldStatus: %{public}u",
        static_cast<uint32_t>(foldStatus));
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr || env_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] this listener or env is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(aniCallbackMtx_);
    if (aniCallback_.empty()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnFoldStatusChanged not register!");
        return;
    }
    if (aniCallback_.find(EVENT_FOLD_STATUS_CHANGED) == aniCallback_.end()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnFoldStatusChanged not this event, return");
        return;
    }
    if (env_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    auto it = aniCallback_.find(EVENT_FOLD_STATUS_CHANGED);
    for (auto oneAniCallback : it->second) {
        auto task = [env = env_, oneAniCallback, foldStatus] () {
            DisplayAniUtils::CallAniFunctionVoid(env, "L@ohos/display/display;", "foldStatusCallback",
                nullptr, oneAniCallback, static_cast<ani_int>(foldStatus));
        };
        if (!eventHandler_) {
            TLOGE(WmsLogTag::DMS, "[ANI] get main event handler failed!");
            return;
        }
        eventHandler_->PostTask(task, "dms:AniDisplayListener::FoldStatusChangedCallback", 0,
            AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
}
void DisplayAniListener::OnFoldAngleChanged(std::vector<float> foldAngles)
{
    TLOGI(WmsLogTag::DMS, "[ANI] called");
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr || env_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] this listener or env is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(aniCallbackMtx_);
    if (aniCallback_.empty()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnFoldAngleChanged not register!");
        return;
    }
    if (aniCallback_.find(EVENT_FOLD_ANGLE_CHANGED) == aniCallback_.end()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnFoldAngleChanged not this event, return");
        return;
    }
    if (env_ != nullptr) {
        auto it = aniCallback_.find(EVENT_FOLD_ANGLE_CHANGED);
        ani_array_double cbArray;
        DisplayAniUtils::CreateAniArrayDouble(env_, foldAngles.size(), &cbArray, foldAngles);
        for (auto oneAniCallback : it->second) {
            auto task = [env = env_, oneAniCallback, cbArray] () {
                DisplayAniUtils::CallAniFunctionVoid(env, "L@ohos/display/display;", "foldAngleChangeCallback",
                    nullptr, oneAniCallback, cbArray);
            };
            if (!eventHandler_) {
                TLOGE(WmsLogTag::DMS, "[ANI] get main event handler failed!");
                return;
            }
            eventHandler_->PostTask(task, "dms:AniDisplayListener::FoldAngleChangeCallback", 0,
                AppExecFwk::EventQueue::Priority::IMMEDIATE);
        }
    } else {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
    }
}
void DisplayAniListener::OnCaptureStatusChanged(bool isCapture)
{
    TLOGI(WmsLogTag::DMS, "[ANI] called");
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr || env_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] this listener or env is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(aniCallbackMtx_);
    if (aniCallback_.empty()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnCaptureStatusChanged not register!");
        return;
    }
    if (aniCallback_.find(EVENT_CAPTURE_STATUS_CHANGED) == aniCallback_.end()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnCaptureStatusChanged not this event, return");
        return;
    }
    if (env_ != nullptr) {
        auto it = aniCallback_.find(EVENT_CAPTURE_STATUS_CHANGED);
        for (auto oneAniCallback : it->second) {
            auto task = [env = env_, oneAniCallback, isCapture] () {
                DisplayAniUtils::CallAniFunctionVoid(env, "L@ohos/display/display;", "captureStatusChangedCallback",
                    nullptr, oneAniCallback, isCapture);
            };
            if (!eventHandler_) {
                TLOGE(WmsLogTag::DMS, "[ANI] get main event handler failed!");
                return;
            }
            eventHandler_->PostTask(task, "dms:AniDisplayListener::CaptureStatusChangedCallback", 0,
                AppExecFwk::EventQueue::Priority::IMMEDIATE);
        }
    } else {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
    }
}
void DisplayAniListener::OnDisplayModeChanged(FoldDisplayMode foldDisplayMode)
{
    TLOGI(WmsLogTag::DMS, "[ANI] called, foldDisplayMode: %{public}u",
        static_cast<uint32_t>(foldDisplayMode));
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr || env_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] this listener or env is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(aniCallbackMtx_);
    if (aniCallback_.empty()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnDisplayModeChanged not register!");
        return;
    }
    if (aniCallback_.find(EVENT_DISPLAY_MODE_CHANGED) == aniCallback_.end()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnDisplayModeChanged not this event, return");
        return;
    }
    if (env_ != nullptr) {
        auto it = aniCallback_.find(EVENT_DISPLAY_MODE_CHANGED);
        for (auto oneAniCallback : it->second) {
            auto task = [env = env_, oneAniCallback, foldDisplayMode] () {
                DisplayAniUtils::CallAniFunctionVoid(env, "L@ohos/display/display;", "foldDisplayModeCallback",
                    nullptr, oneAniCallback, static_cast<ani_int>(foldDisplayMode));
            };
            if (!eventHandler_) {
                TLOGE(WmsLogTag::DMS, "[ANI] get main event handler failed!");
                return;
            }
            eventHandler_->PostTask(task, "dms:AniDisplayListener::DisplayModeChangedCallback", 0,
                AppExecFwk::EventQueue::Priority::IMMEDIATE);
        }
    } else {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
    }
}
void DisplayAniListener::OnAvailableAreaChanged(DMRect area)
{
    TLOGI(WmsLogTag::DMS, "[ANI] called");
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr || env_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] this listener or env is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(aniCallbackMtx_);
    if (aniCallback_.empty()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnAvailableAreaChanged not register!");
        return;
    }
    if (aniCallback_.find(EVENT_AVAILABLE_AREA_CHANGED) == aniCallback_.end()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnAvailableAreaChanged not this event, return");
        return;
    }
    if (env_ != nullptr) {
        auto it = aniCallback_.find(EVENT_AVAILABLE_AREA_CHANGED);
        for (auto oneAniCallback : it->second) {
            ani_object rectObj = nullptr;
            DisplayAniUtils::ConvertRect(area, rectObj, env_);
            auto task = [env = env_, oneAniCallback, rectObj] () {
                DisplayAniUtils::CallAniFunctionVoid(env, "L@ohos/display/display;", "availableAreaChangedCallback",
                    nullptr, oneAniCallback, rectObj);
            };
            if (!eventHandler_) {
                TLOGE(WmsLogTag::DMS, "[ANI] get main event handler failed!");
                return;
            }
            eventHandler_->PostTask(task, "dms:AniDisplayListener::AvailableAreaChangedCallback", 0,
                AppExecFwk::EventQueue::Priority::IMMEDIATE);
        }
    } else {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
    }
}

ani_status DisplayAniListener::CallAniMethodVoid(ani_object object, const char* cls,
    const char* method, const char* signature, ...)
{
    ani_class aniClass;
    ani_status ret = env_->FindClass(cls, &aniClass);
    if (ret != ANI_OK) {
        return ret;
    }
    ani_method aniMethod;
    ret = env_->Class_FindMethod(aniClass, method, signature, &aniMethod);
    if (ret != ANI_OK) {
        return ret;
    }
    va_list args;
    va_start(args, signature);
    ret = env_->Object_CallMethod_Void(object, aniMethod, args);
    va_end(args);
    return ret;
}
}
}