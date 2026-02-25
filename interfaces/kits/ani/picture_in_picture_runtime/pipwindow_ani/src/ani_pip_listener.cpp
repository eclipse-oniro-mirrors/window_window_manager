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

#include "ani_pip_listener.h"

#include <hitrace_meter.h>

#include "ani_pip_utils.h"

namespace OHOS {
namespace Rosen {
static constexpr const char* ETS_NS = "@ohos.PiPWindow.PiPWindow";
static const std::map<PiPStateChangeReason, std::string> PIP_STATE_CHANGE_REASON_TO_STRING_MAP = {
    { PiPStateChangeReason::REQUEST_START,           "requestStart"       },
    { PiPStateChangeReason::AUTO_START,              "autoStart"          },
    { PiPStateChangeReason::REQUEST_DELETE,          "requestDelete"      },
    { PiPStateChangeReason::PANEL_ACTION_DELETE,     "panelActionDelete"  },
    { PiPStateChangeReason::DRAG_DELETE,             "dragDelete"         },
    { PiPStateChangeReason::PANEL_ACTION_RESTORE,    "panelActionRestore" },
    { PiPStateChangeReason::OTHER,                   "other"              },
};

AniPiPListener::~AniPiPListener()
{
    TLOGI(WmsLogTag::WMS_PIP, "~AniPiPListener");
    ani_status ret = vm_->GetEnv(ANI_VERSION_1, &env_);
    if (ret != ANI_OK || env_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "GetEnv failed, ret:%{public}u", ret);
        return;
    }
    if (!env_ || !aniCallback_) {
        TLOGI(WmsLogTag::WMS_PIP, "env_ or aniCallback_ is nullptr, skip");
        return;
    }
    if ((ret = env_->GlobalReference_Delete(aniCallback_)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "GlobalReference_Delete aniCallback_ failed");
        return;
    }
    aniCallback_ = nullptr;
    TLOGI(WmsLogTag::WMS_PIP, "~AniPiPListener finish");
}

void AniPiPListener::SetMainEventHandler()
{
    auto mainRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (mainRunner == nullptr) {
        return;
    }
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainRunner);
}

std::string AniPiPListener::GetStringByStateChangeReason(const PiPStateChangeReason& reasonCode)
{
    auto iter = PIP_STATE_CHANGE_REASON_TO_STRING_MAP.find(reasonCode);
    if (iter != PIP_STATE_CHANGE_REASON_TO_STRING_MAP.end()) {
        return iter->second;
    }
    TLOGE(WmsLogTag::WMS_PIP, "[pip]No mapping found for reasonCode: %{public}d", static_cast<int32_t>(reasonCode));
    return "other";
}

void AniPiPListener::OnPipStateChangeDispatch(const PiPState& state, const PiPStateChangeReason& reason)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    std::string reasonString = GetStringByStateChangeReason(reason);
    auto task = [self = weakRef_, vm = vm_, state, reasonString]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[PIP]AniPiPListener::OnPipStateChangeDispatch");
        auto thisListener = self.promote();
        if (!thisListener || !vm) {
            TLOGE(WmsLogTag::WMS_PIP, "[pip]listener or vm is nullptr");
            return;
        }
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "[pip]GetEnv failed, ret:%{public}u", ret);
            return;
        }
        ani_string reasonStringAni;
        GetAniString(env, reasonString, &reasonStringAni);
        CallAniFunctionVoid(env, ETS_NS, "runOnStateChangeEvent", nullptr,
                            thisListener->aniCallback_, state, reasonStringAni);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_PIP, "[pip]main event handler null");
        return;
    }
    eventHandler_->PostTask(task, "AniPiPListener::OnPipStateChangeDispatch", 0,
                            AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniPiPListener::OnPipStateChangeDispatchError(const int32_t& errorCode)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    PiPState state = PiPState::ERROR;
    std::string reasonString = std::to_string(errorCode);
    auto task = [self = weakRef_, vm = vm_, state, reasonString]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[PIP]AniPiPListener::OnPipStateChangeDispatchError");
        auto thisListener = self.promote();
        if (!thisListener || !vm) {
            TLOGE(WmsLogTag::WMS_PIP, "[pip]listener or vm is nullptr");
            return;
        }
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "[pip]GetEnv failed, ret:%{public}u", ret);
            return;
        }
        CallAniFunctionVoid(env, ETS_NS, "runOnStateChangeEvent", nullptr, thisListener->aniCallback_,
                            state, reasonString.c_str());
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_PIP, "[pip]main event handler null");
        return;
    }
    eventHandler_->PostTask(task, "AniPiPListener::OnPipStateChangeDispatchError", 0,
                            AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

// ===== IPiPLifeCycle =====
void AniPiPListener::OnPreparePictureInPictureStart(PiPStateChangeReason reason)
{
    OnPipStateChangeDispatch(PiPState::ABOUT_TO_START, reason);
}

void AniPiPListener::OnPictureInPictureStart(PiPStateChangeReason reason)
{
    OnPipStateChangeDispatch(PiPState::STARTED, reason);
}

void AniPiPListener::OnPreparePictureInPictureStop(PiPStateChangeReason reason)
{
    OnPipStateChangeDispatch(PiPState::ABOUT_TO_STOP, reason);
}

void AniPiPListener::OnPictureInPictureStop(PiPStateChangeReason reason)
{
    OnPipStateChangeDispatch(PiPState::STOPPED, reason);
}

void AniPiPListener::OnRestoreUserInterface(PiPStateChangeReason reason)
{
    OnPipStateChangeDispatch(PiPState::ABOUT_TO_RESTORE, reason);
}

void AniPiPListener::OnPictureInPictureOperationError(int32_t errorCode)
{
    OnPipStateChangeDispatchError(errorCode);
}

// ===== IPiPActionObserver =====
void AniPiPListener::OnActionEvent(const std::string& actionEvent, int32_t statusCode)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    auto task = [self = weakRef_, vm = vm_, actionEvent, statusCode]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[PIP]AniPiPListener::OnActionEvent");
        auto thisListener = self.promote();
        if (!thisListener || !vm) {
            TLOGE(WmsLogTag::WMS_PIP, "listener or vm is nullptr");
            return;
        }
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "GetEnv failed ret:%{public}u", ret);
            return;
        }
        ani_string act;
        env->String_NewUTF8(actionEvent.c_str(), static_cast<ani_size>(actionEvent.size()), &act);

        CallAniFunctionVoid(env, ETS_NS, "runOnControlPanelActionEvent", nullptr,
                            thisListener->aniCallback_, act, static_cast<ani_int>(statusCode));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_PIP, "main event handler null");
        return;
    }
    eventHandler_->PostTask(task, "AniPiPListener::OnActionEvent", 0,
                            AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

// ===== IPiPControlObserver =====
void AniPiPListener::OnControlEvent(PiPControlType controlType, PiPControlStatus statusCode)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    auto task = [self = weakRef_, vm = vm_, controlType, statusCode]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[PIP]AniPiPListener::OnControlEvent");
        auto thisListener = self.promote();
        if (!thisListener || !vm) {
            TLOGE(WmsLogTag::WMS_PIP, "listener or vm is nullptr");
            return;
        }
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "GetEnv failed ret:%{public}u", ret);
            return;
        }
        CallAniFunctionVoid(env, ETS_NS, "runOnControlEvent", nullptr,
                            thisListener->aniCallback_,
                            static_cast<ani_int>(controlType),
                            static_cast<ani_int>(statusCode));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_PIP, "main event handler null");
        return;
    }
    eventHandler_->PostTask(task, "AniPiPListener::OnControlEvent", 0,
                            AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

// ===== IPiPWindowSize =====
void AniPiPListener::OnPipSizeChange(const PiPWindowSize& size)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    auto task = [self = weakRef_, vm = vm_, size]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[PIP]AniPiPListener::OnPipSizeChange");
        auto thisListener = self.promote();
        if (!thisListener || !vm) {
            TLOGE(WmsLogTag::WMS_PIP, "listener or vm is nullptr");
            return;
        }
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "GetEnv failed ret:%{public}d", (int)ret);
            return;
        }
        CallAniFunctionVoid(env, ETS_NS, "runOnPipWindowSizeChangeEvent", nullptr,
                            thisListener->aniCallback_,
                            static_cast<ani_int>(size.width),
                            static_cast<ani_int>(size.height),
                            static_cast<ani_double>(size.scale));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_PIP, "main event handler null");
        return;
    }
    eventHandler_->PostTask(task, "AniPiPListener::OnPipSizeChange", 0,
                            AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

// ===== IPiPActiveStatusObserver =====
void AniPiPListener::OnActiveStatusChange(const bool& status)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    auto task = [self = weakRef_, vm = vm_, status]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[PIP]AniPiPListener::OnActiveStatusChange");
        auto thisListener = self.promote();
        if (!thisListener || !vm) {
            TLOGE(WmsLogTag::WMS_PIP, "listener or vm is nullptr");
            return;
        }
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "GetEnv failed ret:%{public}d", (int)ret);
            return;
        }
        CallAniFunctionVoid(env, ETS_NS, "runOnActiveStatusChangeEvent", nullptr, thisListener->aniCallback_, status);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_PIP, "main event handler null");
        return;
    }
    eventHandler_->PostTask(task, "AniPiPListener::OnActiveStatusChange", 0,
                            AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

} // namespace Rosen
} // namespace OHOS