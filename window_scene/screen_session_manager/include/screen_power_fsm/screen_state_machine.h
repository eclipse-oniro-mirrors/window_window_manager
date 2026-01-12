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

#ifndef SCREEN_STATE_MACHINE_H
#define SCREEN_STATE_MACHINE_H

#include <iostream>
#include <map>

#include <screen_manager/screen_types.h>
#include "dm_common.h"
#include "wm_single_instance.h"
#include "screen_power_fsm/session_display_power_controller.h"
#include "screen_power_fsm/screen_state_timer.h"

const int INVALID_STATE = -1;

namespace OHOS::Rosen {
enum class ScreenTransitionState {
    SCREEN_ON,
    SCREEN_OFF,
    WAIT_SCREEN_ON_READY,
    WAIT_SCREEN_ADVANCED_ON_READY,
    SCREEN_AOD,
    WAIT_LOCK_SCREEN_IND,
    WAIT_SCREEN_CTRL_RSP,
    SCREEN_ADVANCED_ON,
    SCREEN_DOZE,
    SCREEN_DOZE_SUSPEND,
    SCREEN_INIT,
    TRANSITION_STATE_MAX,
};

enum class ScreenPowerEvent {
    POWER_ON,
    POWER_OFF,
    POWER_INIT,
    POWER_ON_DIRECTLY,
    POWER_OFF_DIRECTLY,
    SUSPEND,
    SET_SCREEN_POWER_FOR_ALL_POWER_ON,
    SET_SCREEN_POWER_FOR_ALL_POWER_OFF,
    OFF_CANCEL,
    SCREEN_MANAGE_RIGHT_REQ,
    AOD_ENTER_SUCCESS,
    AOD_ENTER_FAIL,
    SET_DISPLAY_STATE,
    SET_DISPLAY_STATE_DOZE,
    SET_DISPLAY_STATE_DOZE_SUSPEND,
    SET_SCREEN_POWER,
    E_ADVANCED_ON,
    E_ADVANCED_OFF,
    E_DOZE_SUSPEND,
    E_DOZE,
    DMS_POWER_CB_END,
    WAKEUP_BEGIN,
    SUSPEND_BEGIN,
    WAKEUP_BEGIN_ADVANCED,
    SCREEN_POWER_EVENT_MAX,
    SYNC_POWER_ON, //Special scenario, PMS force synchronization ON state
};

enum class AodStatus {
    UNKNOWN,
    SUCCESS,
    FAILURE,
    AOD_STATUS_MAX,
};

using StateEvent = std::pair<ScreenTransitionState, ScreenPowerEvent>;
using ScreenPowerInfoType = std::variant<std::monostate, PowerStateChangeReason,
    DisplayState, std::pair<ScreenPowerState, PowerStateChangeReason>, std::pair<ScreenId, ScreenPowerStatus>,
    std::pair<DisplayId, DisplayState>>;
using Action = std::function<bool (ScreenPowerEvent, const ScreenPowerInfoType&)>;

struct Transition {
    ScreenTransitionState targetState;
    Action action;
    int32_t timeout = 0;
    Action timeoutCallback = nullptr;
    bool isValid = true;
};

class ScreenStateMachine : public RefBase {
WM_DECLARE_SINGLE_INSTANCE_BASE(ScreenStateMachine)
public:
    ScreenStateMachine();
    virtual ~ScreenStateMachine() = default;
    void InitStateMachineTbl();
    void InitStateMachine(uint32_t refCnt);
    bool HandlePowerStateChange(ScreenPowerEvent event, const ScreenPowerInfoType& type);
    Transition& GetTransition(ScreenTransitionState state, ScreenPowerEvent event);
    ScreenTransitionState GetTransitionState();
    void SetTransitionState(ScreenTransitionState state);
    bool WaitForBlock(int32_t delay);
    void NotifyPowerBlock();
    void IncScreenStateInitRef();
    void ToTransition(ScreenTransitionState state, bool isForce);
    ScreenPowerStatus GetCurrentPowerStatus();
    void SetCurrentPowerStatus(ScreenPowerStatus status);

private:
    std::atomic<ScreenTransitionState> transState_;
    std::atomic<ScreenPowerStatus> powerStatus_;
    std::map<StateEvent, Transition> stateMachine_;
    void TransferState(ScreenTransitionState state);
    static bool DoWakeUpBegin(ScreenPowerEvent event, const ScreenPowerInfoType& type);
    static bool DoSetDisplayState(ScreenPowerEvent event, const ScreenPowerInfoType& type);
    static bool DoScreenPowerOff(ScreenPowerEvent event, const ScreenPowerInfoType& type);
    static bool DoScreenPowerOn(ScreenPowerEvent event, const ScreenPowerInfoType& type);
    static bool DoSetScreenPower(ScreenPowerEvent event, const ScreenPowerInfoType& type);
    static bool DoRecordTransNormal(ScreenPowerEvent event, const ScreenPowerInfoType& type);
    static bool DoAodExitAndSetPowerOn(ScreenPowerEvent event, const ScreenPowerInfoType& type);
    static bool DoAodExitAndSetPowerOff(ScreenPowerEvent event, const ScreenPowerInfoType& type);
    static bool DoAodExitAndSetPowerAllOff(ScreenPowerEvent event, const ScreenPowerInfoType& type);
    static bool DoSuspendBegin(ScreenPowerEvent event, const ScreenPowerInfoType& type);
    static bool DoWaitAodRequest(ScreenPowerEvent event, const ScreenPowerInfoType& type);
    static bool DoSetScreenPowerForAll(ScreenPowerEvent event, const ScreenPowerInfoType& type);
    static bool ActionScreenPowerOff(ScreenPowerEvent event, const ScreenPowerInfoType& type);

    ScreenStateTimer timer_;
    std::mutex mtx;
    bool isForceTrans_ = false;
    uint32_t initRefCnt_ = 0;
};
} // namespace OHOS::Rosen
#endif // SCREEN_STATE_MACHINE_H