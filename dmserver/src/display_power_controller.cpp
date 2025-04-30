/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "display_power_controller.h"
#include "display_manager_service.h"
#include "display_manager_agent_controller.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

bool DisplayPowerController::SuspendBegin(PowerStateChangeReason reason)
{
    TLOGI(WmsLogTag::DMS_DMSERVER, "reason:%{public}u", reason);
    std::map<DisplayId, sptr<DisplayInfo>> emptyMap;
    displayStateChangeListener_(DISPLAY_ID_INVALID, nullptr, emptyMap, DisplayStateChangeType::BEFORE_SUSPEND);
    return true;
}

bool DisplayPowerController::SetDisplayState(DisplayState state)
{
    TLOGI(WmsLogTag::DMS_DMSERVER, "state:%{public}u", state);
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (displayState_ == state) {
            TLOGE(WmsLogTag::DMS_DMSERVER, "state is already set");
            return false;
        }
    }
    switch (state) {
        case DisplayState::ON: {
            {
                std::lock_guard<std::recursive_mutex> lock(mutex_);
                displayState_ = state;
            }
            DisplayManagerAgentController::GetInstance().NotifyDisplayPowerEvent(DisplayPowerEvent::DISPLAY_ON,
                EventStatus::BEGIN);
            break;
        }
        case DisplayState::OFF: {
            {
                std::lock_guard<std::recursive_mutex> lock(mutex_);
                displayState_ = state;
            }
            DisplayManagerAgentController::GetInstance().NotifyDisplayPowerEvent(DisplayPowerEvent::DISPLAY_OFF,
                EventStatus::BEGIN);
            break;
        }
        default: {
            TLOGW(WmsLogTag::DMS_DMSERVER, "unknown DisplayState!");
            return false;
        }
    }
    DisplayManagerAgentController::GetInstance().NotifyDisplayStateChanged(DISPLAY_ID_INVALID, state);
    return true;
}

DisplayState DisplayPowerController::GetDisplayState(DisplayId displayId)
{
    return displayState_;
}

void DisplayPowerController::NotifyDisplayEvent(DisplayEvent event)
{
    TLOGI(WmsLogTag::DMS_DMSERVER, "DisplayEvent:%{public}u", event);
    if (event == DisplayEvent::UNLOCK) {
        std::map<DisplayId, sptr<DisplayInfo>> emptyMap;
        displayStateChangeListener_(DISPLAY_ID_INVALID, nullptr, emptyMap, DisplayStateChangeType::BEFORE_UNLOCK);
        DisplayManagerAgentController::GetInstance().NotifyDisplayPowerEvent(DisplayPowerEvent::DESKTOP_READY,
            EventStatus::BEGIN);
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        isKeyguardDrawn_ = false;
        return;
    }
    if (event == DisplayEvent::KEYGUARD_DRAWN) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        isKeyguardDrawn_ = true;
    }
}
}
}