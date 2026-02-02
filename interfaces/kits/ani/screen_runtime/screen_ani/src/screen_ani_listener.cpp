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

#include "screen_ani_listener.h"

#include "screen_ani_utils.h"
#include "window_manager_hilog.h"
 
namespace OHOS {
namespace Rosen {

const std::string ANI_EVENT_CONNECT = "connect";
const std::string ANI_EVENT_DISCONNECT = "disconnect";
const std::string ANI_EVENT_CHANGE = "change";
 
ScreenAniListener::~ScreenAniListener()
{
    TLOGI(WmsLogTag::DMS, "[ANI]~ScreenAniListener");
}

void ScreenAniListener::SetMainEventHandler()
{
    auto mainRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (mainRunner == nullptr) {
        return;
    }
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainRunner);
}
 
void ScreenAniListener::AddCallback(const std::string& type, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] AddCallback is called, type = %{public}s", type.c_str());
    if (env_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    aniCallback_[type].emplace_back(callback);
    TLOGI(WmsLogTag::DMS, "[ANI] AddCallback success aniCallback_ size: %{public}u!",
        static_cast<uint32_t>(aniCallback_[type].size()));
}
void ScreenAniListener::RemoveAllCallback()
{
    std::lock_guard<std::mutex> lock(mtx_);
    aniCallback_.clear();
}
void ScreenAniListener::RemoveCallback(ani_env* env, const std::string& type, ani_ref callback)
{
    std::lock_guard<std::mutex> lock(mtx_);
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
            iter = listeners.erase(iter);
        } else {
            iter++;
        }
    }
    TLOGI(WmsLogTag::DMS, "[ANI] Success remove callback, jsCallBack_ size: %{public}u!",
        static_cast<uint32_t>(listeners.size()));
}
void ScreenAniListener::OnConnect(ScreenId id)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin, displayId: %{public}" PRIu64, id);
    std::lock_guard<std::mutex> lock(mtx_);
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] this listener is nullptr");
        return;
    }
    if (aniCallback_.empty()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnConnect not register!");
        return;
    }
    auto it = aniCallback_.find(ANI_EVENT_CONNECT);
    if (it == aniCallback_.end()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnConnect not this event, return");
        return;
    }
    std::vector<ani_ref> vec = it->second;
    TLOGI(WmsLogTag::DMS, "vec_callback size: %{public}d", vec.size());
    // find callbacks in vector
    for (auto oneAniCallback : vec) {
        if (env_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "[ANI] null env");
            return;
        }
        ani_boolean undefRes = 0;
        env_->Reference_IsUndefined(oneAniCallback, &undefRes);
        // judge is null or undefined
        if (undefRes) {
            TLOGE(WmsLogTag::DMS, "[ANI] oneAniCallback is undef");
            continue;
        }

        auto task = [env = env_, oneAniCallback, id] {
            ScreenAniUtils::CallAniFunctionVoid(env, "@ohos.screen.screen", "screenEventCallBack",
                "C{std.core.Object}l:", oneAniCallback, static_cast<ani_long>(id));
        };
        if (!eventHandler_) {
            TLOGE(WmsLogTag::DMS, "[ANI] get main event handler failed!");
            return;
        }
        eventHandler_->PostTask(std::move(task), "dms:AniScreenListener::ConnectCallBack", 0,
            AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
}
void ScreenAniListener::OnDisconnect(ScreenId id)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    std::lock_guard<std::mutex> lock(mtx_);
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] this listener is nullptr");
        return;
    }
    TLOGI(WmsLogTag::DMS, "[ANI] OnDisconnect is called, displayId: %{public}" PRIu64, id);
    if (aniCallback_.empty()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnDisconnect not register!");
        return;
    }
    auto it = aniCallback_.find(ANI_EVENT_DISCONNECT);
    if (it == aniCallback_.end()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnDisconnect not this event, return");
        return;
    }
    std::vector<ani_ref> vec = it->second;
    TLOGI(WmsLogTag::DMS, "vec_callback size: %{public}d", vec.size());
    // find callbacks in vector
    for (auto oneAniCallback : vec) {
        if (env_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "[ANI] null env");
            return;
        }
        ani_boolean undefRes = 0;
        env_->Reference_IsUndefined(oneAniCallback, &undefRes);
        // judge is null or undefined
        if (undefRes) {
            TLOGE(WmsLogTag::DMS, "[ANI] oneAniCallback is undef");
            continue;
        }

        auto task = [env = env_, oneAniCallback, id] {
            ScreenAniUtils::CallAniFunctionVoid(env, "@ohos.screen.screen", "screenEventCallBack",
                "C{std.core.Object}l:", oneAniCallback, static_cast<ani_long>(id));
        };
        if (!eventHandler_) {
            TLOGE(WmsLogTag::DMS, "[ANI] get main event handler failed!");
            return;
        }
        eventHandler_->PostTask(std::move(task), "dms:AniScreenListener::DisconnectCallBack", 0,
            AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
}

void ScreenAniListener::OnChange(ScreenId id)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    std::lock_guard<std::mutex> lock(mtx_);
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] this listener is nullptr");
        return;
    }
    TLOGI(WmsLogTag::DMS, "[ANI] OnChange is called, displayId: %{public}" PRIu64, id);
    if (aniCallback_.empty()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnChange not register!");
        return;
    }
    auto it = aniCallback_.find(ANI_EVENT_CHANGE);
    if (it == aniCallback_.end()) {
        TLOGE(WmsLogTag::DMS, "[ANI] OnChange not this event, return");
        return;
    }
    std::vector<ani_ref> vec = it->second;
    // find callbacks in vector
    for (auto oneAniCallback : vec) {
        if (env_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "[ANI] null env");
            return;
        }
        ani_boolean undefRes = 0;
        env_->Reference_IsUndefined(oneAniCallback, &undefRes);
        if (undefRes) {
            TLOGE(WmsLogTag::DMS, "[ANI] oneAniCallback undefRes, continue");
            continue;
        }
        auto task = [env = env_, oneAniCallback, id] () {
            ScreenAniUtils::CallAniFunctionVoid(env, "@ohos.screen.screen", "screenEventCallBack",
                "C{std.core.Object}l:", oneAniCallback, static_cast<ani_long>(id));
        };
        if (!eventHandler_) {
            TLOGE(WmsLogTag::DMS, "get main event handler failed!");
            return;
        }
        eventHandler_->PostTask(task, "dms:AniScreenListener::CreateCallBack", 0,
            AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
}
 
ani_status ScreenAniListener::CallAniMethodVoid(ani_object object, const char* cls,
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
