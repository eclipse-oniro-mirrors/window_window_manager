/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
 * Licensed under * Apache License, Version 2.0 (the "License");
 * you may not use this file except * compliance with * License.
 * You may obtain a copy of * License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under * License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See * License for * specific language governing permissions and
 * limitations under * License.
 */

#include "ani_fv_window_listener.h"

#include "ani_fv_window_utils.h"

#include <hitrace_meter.h>
#include "event_handler.h"
#include "event_runner.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

AniFvWindowListener::~AniFvWindowListener()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]~AniFvWindowListener destroyed");
    ani_status ret = ANI_OK;
    if (env_ == nullptr || aniCallback_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]env_ or aniCallback_ is nullptr,skip");
        return;
    }
    if ((ret = env_->GlobalReference_Delete(aniCallback_)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]GlobalReference_Delete failed");
        return;
    }
    aniCallback_ = nullptr;
}

void AniFvWindowListener::SetMainEventHandler()
{
    auto mainRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (mainRunner == nullptr) {
        return;
    }
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainRunner);
}

void AniFvWindowListener::OnStateChange(const FloatViewState& state, const std::string& stopReason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]called, onStateChange");
    auto task = [self = weakRef_, vm = vm_, state, stopReason] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[FV]AniFvWindowListener::onStateChange");
        // check Listener
        auto thisListener = self.promote();
        if (thisListener == nullptr || vm == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]this listener or vm is nullptr");
            return;
        }
        // get env
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]Get env failed, ret:%{public}u", ret);
            return;
        }
        ani_object infoObj = AniFvUtils::CreateAniFloatViewStateChangeInfoObject(env, state, stopReason);
        ani_boolean isUndefined = false;
        env->Reference_IsUndefined(infoObj, &isUndefined);
        if (isUndefined) {
            return;
        }
        if (thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]callback has been destroyed");
            return;
        }
        AniFvUtils::CallAniFunctionVoid(env, "@ohos.window.floatView.floatView", "runOnStateChangeEvent",
            nullptr, thisListener->aniCallback_, infoObj);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task,
        "AniFvWindowListener::OnStateChange", 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniFvWindowListener::OnRectangleChange(const Rect& window, double scale, const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]called, onRectChange");
    auto task = [self = weakRef_, vm = vm_, window, scale, reason] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[FV]AniFvWindowListener::onRectChange");
        // check Listener
        auto thisListener = self.promote();
        if (thisListener == nullptr || vm == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]this listener or vm is nullptr");
            return;
        }
        // get env
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]Get env failed, ret:%{public}u", ret);
            return;
        }
        // working
        ani_object rectObj = AniFvUtils::CreateAniFloatViewRectChangeInfoObject(env, window, scale, reason);
        ani_boolean isUndefined = false;
        env->Reference_IsUndefined(rectObj, &isUndefined);
        if (isUndefined) {
            return;
        }
        if (thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]callback has been destroyed");
            return;
        }
        AniFvUtils::CallAniFunctionVoid(env, "@ohos.window.floatView.floatView", "runOnRectChangeEvent",
            nullptr, thisListener->aniCallback_, rectObj);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task,
        "AniFvWindowListener::OnRectangleChange", 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniFvWindowListener::OnLimitsChange(const FloatViewLimits& limits)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]called, onLimitsChange");
    auto task = [self = weakRef_, vm = vm_, limits] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[FV]AniFvWindowListener::onLimitsChange");
        // check Listener
        auto thisListener = self.promote();
        if (thisListener == nullptr || vm == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]this listener or vm is nullptr");
            return;
        }
        // get env
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]Get env failed, ret:%{public}u", ret);
            return;
        }
        // working
        ani_object limitsObj = AniFvUtils::CreateAniFloatViewLimitsObject(env, limits);
        ani_boolean isUndefined = false;
        env->Reference_IsUndefined(limitsObj, &isUndefined);
        if (isUndefined) {
            return;
        }
        if (thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]callback has been destroyed");
            return;
        }
        AniFvUtils::CallAniFunctionVoid(env, "@ohos.window.floatView.floatView", "runOnLimitsChangeEvent",
            nullptr, thisListener->aniCallback_, limitsObj);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task,
        "AniFvWindowListener::OnLimitsChange", 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

}  // namespace Rosen
}  // namespace OHOS