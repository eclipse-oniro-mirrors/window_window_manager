/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "window_manager_hilog.h"
#include "ani_fb_window_controller.h"
#include "ani_fb_window_utils.h"
#include "ani_fb_window_listener.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

AniFbWindowListener::~AniFbWindowListener()
{
    TLOGI(WmsLogTag::DEFAULT, "[FB]~AniFbWindowListener destroyed");
    ani_status ret = ANI_OK;
    if (env_ == nullptr || aniCallback_ == nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "[FB]env_ or aniCallback_ is nullptr,skip");
        return;
    }
    if ((ret = env_->GlobalReference_Delete(aniCallback_)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]GlobalReference_Delete failed");
        return;
    }
    aniCallback_ = nullptr;
}

void AniFbWindowListener::SetMainEventHandler()
{
    auto mainRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (mainRunner == nullptr) {
        return;
    }
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainRunner);
}

void AniFbWindowListener::OnClickEvent()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]called, onClickEvent");
    auto task = [self = weakRef_, vm = vm_] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[FB]AniFbWindowListener::OnClickEvent");
        // check Listener
        auto thisListener = self.promote();
        if (thisListener == nullptr || vm == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[FB]this listener or vm is nullptr");
            return;
        }
        // get env
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[FB]Get env failed, ret:%{public}u", ret);
            return;
        }
        // working
        CallAniFunctionVoid(env, "@ohos.window.floatingBall.floatingBall", "runOnClickEvent",
                            nullptr, thisListener->aniCallback_);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "AniFbWindowListener::OnClickEvent", 0,
                            AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniFbWindowListener::OnFloatingBallStart() {OnFbListenerCallback(FloatingBallState::STARTED);}

void AniFbWindowListener::OnFloatingBallStop() {OnFbListenerCallback(FloatingBallState::STOPPED);}

void AniFbWindowListener::OnFbListenerCallback(const FloatingBallState& state)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]called, onStateChangeEvent");
    auto task = [self = weakRef_, vm = vm_, state] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[FB]AniFbWindowListener::onStateChangeEvent");
        // check Listener
        auto thisListener = self.promote();
        if (thisListener == nullptr || vm == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[FB]this listener or vm is nullptr");
            return;
        }
        // get env
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[FB]Get env failed, ret:%{public}u", ret);
            return;
        }
        // working
        CallAniFunctionVoid(env, "@ohos.window.floatingBall.floatingBall", "runOnStateChangeEvent",
                            nullptr, thisListener->aniCallback_, state);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "AniFbWindowListener::onStateChangeEvent", 0,
                            AppExecFwk::EventQueue::Priority::IMMEDIATE);
}
} // namespace Rosen
} // namespace OHOS