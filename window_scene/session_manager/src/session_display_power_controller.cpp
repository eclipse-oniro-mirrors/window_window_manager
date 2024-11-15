/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "session_display_power_controller.h"
#include "screen_session_manager.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
bool SessionDisplayPowerController::SuspendBegin(PowerStateChangeReason reason)
{
    TLOGI(WmsLogTag::DMS, "reason:%{public}u", reason);
    std::map<DisplayId, sptr<DisplayInfo>> emptyMap;
    displayStateChangeListener_(DISPLAY_ID_INVALID, nullptr, emptyMap, DisplayStateChangeType::BEFORE_SUSPEND);
    return true;
}

void SessionDisplayPowerController::SetDisplayStateToOn(DisplayState& state)
{
    displayState_ = state;
    if (!ScreenSessionManager::GetInstance().IsMultiScreenCollaboration()) {
        ScreenSessionManager::GetInstance().NotifyDisplayPowerEvent(DisplayPowerEvent::DISPLAY_ON,
            EventStatus::BEGIN, PowerStateChangeReason::STATE_CHANGE_REASON_INIT);
        ScreenSessionManager::GetInstance().BlockScreenOnByCV();
    }
}

bool SessionDisplayPowerController::SetDisplayState(DisplayState state)
{
    TLOGI(WmsLogTag::DMS, "[UL_POWER]state:%{public}u", state);
    switch (state) {
        case DisplayState::ON: {
            SetDisplayStateToOn(state);
            break;
        }
        case DisplayState::OFF: {
            DisplayState lastState = displayState_;
            displayState_ = state;
            if (!ScreenSessionManager::GetInstance().IsMultiScreenCollaboration()) {
                {
                    std::lock_guard<std::mutex> notifyLock(notifyMutex_);
                    canCancelSuspendNotify_ = false;
                    if (needCancelNotify_) {
                        TLOGI(WmsLogTag::DMS, "[UL_POWER]"
                            "SetDisplayState to OFF is canceled successfully before notify");
                        needCancelNotify_ = false;
                        displayState_ = lastState;
                        ScreenSessionManager::GetInstance().NotifyDisplayStateChanged(DISPLAY_ID_INVALID,
                            DisplayState::UNKNOWN);
                        return false;
                    }
                    ScreenSessionManager::GetInstance().NotifyDisplayPowerEvent(DisplayPowerEvent::DISPLAY_OFF,
                        EventStatus::BEGIN, PowerStateChangeReason::STATE_CHANGE_REASON_INIT);
                }
                WaitScreenOffNotify(state);

                if (canceledSuspend_) {
                    TLOGI(WmsLogTag::DMS, "[UL_POWER]SetDisplayState to OFF is canceled successfully after notify");
                    canceledSuspend_ = false;
                    displayState_ = lastState;
                    ScreenSessionManager::GetInstance().NotifyDisplayStateChanged(DISPLAY_ID_INVALID,
                        DisplayState::UNKNOWN);
                    return false;
                }
            }
            break;
        }
        default: {
            TLOGW(WmsLogTag::DMS, "[UL_POWER]unknown DisplayState!");
            return false;
        }
    }
    ScreenSessionManager::GetInstance().NotifyDisplayStateChanged(DISPLAY_ID_INVALID, state);
    return true;
}

void SessionDisplayPowerController::WaitScreenOffNotify(DisplayState& state)
{
    if (!ScreenSessionManager::GetInstance().IsPreBrightAuthFail()) {
        ScreenSessionManager::GetInstance().BlockScreenOffByCV();
        if (ScreenSessionManager::GetInstance().IsScreenLockSuspend()) {
            state = DisplayState::ON_SUSPEND;
        }
    }
}


DisplayState SessionDisplayPowerController::GetDisplayState(DisplayId displayId)
{
    return displayState_;
}

void SessionDisplayPowerController::NotifyDisplayEvent(DisplayEvent event)
{
    TLOGI(WmsLogTag::DMS, "[UL_POWER]DisplayEvent:%{public}u", event);
    if (event == DisplayEvent::UNLOCK) {
        std::map<DisplayId, sptr<DisplayInfo>> emptyMap;
        displayStateChangeListener_(DISPLAY_ID_INVALID, nullptr, emptyMap, DisplayStateChangeType::BEFORE_UNLOCK);
        ScreenSessionManager::GetInstance().NotifyDisplayPowerEvent(DisplayPowerEvent::DESKTOP_READY,
            EventStatus::BEGIN, PowerStateChangeReason::STATE_CHANGE_REASON_INIT);
        return;
    }
}
}
}