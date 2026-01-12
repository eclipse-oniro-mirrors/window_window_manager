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

#include "screen_power_fsm/screen_state_machine.h"
#include "window_manager_hilog.h"
#include "screen_session_manager.h"

namespace OHOS::Rosen {

const uint32_t SCREEN_STATE_NORMAL_TIMEOUT_MS = 1000;
const uint32_t SCREEN_STATE_AOD_TIMEOUT_MS = 3000;

WM_IMPLEMENT_SINGLE_INSTANCE(ScreenStateMachine)

ScreenStateMachine::ScreenStateMachine()
{
    SetTransitionState(ScreenTransitionState::SCREEN_INIT);
}

void ScreenStateMachine::InitStateMachine(uint32_t refCnt)
{
    initRefCnt_ = refCnt;
    if (refCnt == 0) {
        TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] Init end, set transition state to SCREEN_ON");
        SetTransitionState(ScreenTransitionState::SCREEN_ON);
    }
    InitStateMachineTbl();
}

// only used during FSM
void ScreenStateMachine::ToTransition(ScreenTransitionState state, bool isForce)
{
    isForceTrans_ = isForce;
    SetTransitionState(state);
}

void ScreenStateMachine::IncScreenStateInitRef()
{
    static uint32_t initRef = 0;
    TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] Enter");
    if (++initRef == initRefCnt_) {
        TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] Init end, set transition state to SCREEN_ON");
        SetTransitionState(ScreenTransitionState::SCREEN_ON);
    }
}

void ScreenStateMachine::SetTransitionState(ScreenTransitionState state)
{
    TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] from %{public}u to %{public}u", GetTransitionState(), state);
    transState_.store(state);
}

ScreenTransitionState ScreenStateMachine::GetTransitionState()
{
    return transState_.load();
}

void ScreenStateMachine::TransferState(ScreenTransitionState targetState)
{
    TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] from %{public}u to %{public}u", GetTransitionState(), targetState);
    SetTransitionState(targetState);
}

void ScreenStateMachine::SetCurrentPowerStatus(ScreenPowerStatus status)
{
    powerStatus_.store(status);
}

ScreenPowerStatus ScreenStateMachine::GetCurrentPowerStatus()
{
    return powerStatus_.load();
}

bool ScreenStateMachine::HandlePowerStateChange(ScreenPowerEvent event, const ScreenPowerInfoType& type)
{
    std::unique_lock lock(mtx);
    std::function<bool (ScreenPowerEvent, const ScreenPowerInfoType&)> action;
    ScreenTransitionState state = GetTransitionState();
    bool ret = 0;

    TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] Enter, currentState: %{public}u, event: %{public}u", state, event);
    Transition& transition = GetTransition(state, event);
    if (!transition.isValid) {
        TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] invalid transition");
        return false;
    }
    timer_.StopTimer(static_cast<int32_t>(state));
    action = transition.action;
    if (action) {
        ret = action(event, type);
        if (transition.timeout > 0 && transition.timeoutCallback) {
            timer_.StartTimer(static_cast<int32_t>(transition.targetState),
                transition.timeout, [this, &transition, event, type] {
                    transition.timeoutCallback(event, type);
                });
        }
        if (!isForceTrans_ && GetTransitionState() == state) {
            TransferState(transition.targetState);
        }
        isForceTrans_ = false;
        if (transition.timeout <= 0) {
            timer_.StopTimer(static_cast<int32_t>(transition.targetState));
        }
    }
    return ret;
}

bool ScreenStateMachine::DoWakeUpBegin(ScreenPowerEvent event, const ScreenPowerInfoType& type)
{
    TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] event: %{public}u", event);
    return ScreenSessionManager::GetInstance().DoWakeUpBegin(std::get<PowerStateChangeReason>(type));
}

bool ScreenStateMachine::DoSuspendBegin(ScreenPowerEvent event, const ScreenPowerInfoType& type)
{
    TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] event: %{public}u", event);
    return ScreenSessionManager::GetInstance().DoSuspendBegin(std::get<PowerStateChangeReason>(type));
}

bool ScreenStateMachine::DoSetDisplayState(ScreenPowerEvent event, const ScreenPowerInfoType& type)
{
    TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] event: %{public}u", event);
    return ScreenSessionManager::GetInstance().DoSetDisplayState(std::get<DisplayState>(type));
}

bool ScreenStateMachine::DoScreenPowerOn(ScreenPowerEvent event, const ScreenPowerInfoType& type)
{
    auto params = std::get<std::pair<ScreenId, ScreenPowerStatus>>(type);
    ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(params.first, params.second);
    return true;
}

bool ScreenStateMachine::DoSetScreenPower(ScreenPowerEvent event, const ScreenPowerInfoType& type)
{
    auto params = std::get<std::pair<ScreenId, ScreenPowerStatus>>(type);
    if (!ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(params.first, params.second)) {
        TLOGW(WmsLogTag::DMS, "[ScreenPower FSM] Set Screen power status failed");
        return false;
    }
    ScreenStateMachine::GetInstance().SetCurrentPowerStatus(params.second);
    return true;
}

bool ScreenStateMachine::DoRecordTransNormal(ScreenPowerEvent event, const ScreenPowerInfoType& type)
{
    TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] event: %{public}u", event);
    return true;
}

bool ScreenStateMachine::DoSetScreenPowerForAll(ScreenPowerEvent event, const ScreenPowerInfoType& type)
{
    TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] event: %{public}u", event);
    auto* params = std::get_if<std::pair<ScreenPowerState, PowerStateChangeReason>>(&type);
    if (!params) {
        TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] invalid params");
        return false;
    }
    return ScreenSessionManager::GetInstance().SetScreenPowerForAll(params->first, params->second);
}

bool ScreenStateMachine::DoAodExitAndSetPowerOn(ScreenPowerEvent event, const ScreenPowerInfoType& type)
{
    auto params = std::get<std::pair<ScreenId, ScreenPowerStatus>>(type);
    ScreenSessionManager::GetInstance().DoAodExitAndSetPower(params.first, ScreenPowerStatus::POWER_STATUS_ON);
    return true;
}

bool ScreenStateMachine::DoAodExitAndSetPowerOff(ScreenPowerEvent event, const ScreenPowerInfoType& type)
{
    auto* pair_ptr = std::get_if<std::pair<ScreenId, ScreenPowerStatus>>(&type);
    if (pair_ptr) {
        ScreenSessionManager::GetInstance().DoAodExitAndSetPower(pair_ptr->first, pair_ptr->second);
        return true;
    }
    return false;
}

bool ScreenStateMachine::DoAodExitAndSetPowerAllOff(ScreenPowerEvent event, const ScreenPowerInfoType& type)
{
    TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] event: %{public}u", event);
    ScreenSessionManager::GetInstance().DoAodExitAndSetPowerAllOff();
    ScreenStateMachine::GetInstance().SetTransitionState(ScreenTransitionState::SCREEN_OFF);
    return true;
}

bool ScreenStateMachine::ActionScreenPowerOff(ScreenPowerEvent event, const ScreenPowerInfoType& type)
{
    bool ret = DoSetScreenPower(event, type);
    if (!ret) {
        return false;
    }
    ScreenPowerState state = ScreenSessionManager::GetInstance().GetScreenPower();
    if (state != ScreenPowerState::POWER_OFF) {
        ScreenTransitionState currState = ScreenStateMachine::GetInstance().GetTransitionState();
        ScreenStateMachine::GetInstance().ToTransition(currState, true);
        TLOGI(WmsLogTag::DMS, "[ScreenPower FSM] main screen is on, stay current state: %{public}d event: %{public}u",
            currState, event);
    }
    return true;
}

Transition& ScreenStateMachine::GetTransition(ScreenTransitionState state, ScreenPowerEvent event)
{
    static Transition errorTransition;
    errorTransition.isValid = false;
    auto it = stateMachine_.find({state, event});
    return (it != stateMachine_.end()) ? it->second : errorTransition;
}

void ScreenStateMachine::InitStateMachineTbl()
{
    stateMachine_[{ScreenTransitionState::SCREEN_INIT, ScreenPowerEvent::POWER_ON_DIRECTLY}] = {
        ScreenTransitionState::SCREEN_INIT, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::SCREEN_INIT, ScreenPowerEvent::POWER_OFF_DIRECTLY}] = {
        ScreenTransitionState::SCREEN_INIT, &ScreenStateMachine::DoSetScreenPower};

    stateMachine_[{ScreenTransitionState::SCREEN_OFF, ScreenPowerEvent::WAKEUP_BEGIN}] = {
        ScreenTransitionState::WAIT_SCREEN_ON_READY, &ScreenStateMachine::DoWakeUpBegin};
    stateMachine_[{ScreenTransitionState::SCREEN_OFF, ScreenPowerEvent::POWER_ON_DIRECTLY}] = {
        ScreenTransitionState::SCREEN_ON, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::SCREEN_OFF, ScreenPowerEvent::POWER_OFF_DIRECTLY}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::SCREEN_OFF, ScreenPowerEvent::E_ADVANCED_ON}] = {
        ScreenTransitionState::SCREEN_ADVANCED_ON, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::SCREEN_OFF, ScreenPowerEvent::SET_DISPLAY_STATE_DOZE}] = {
        ScreenTransitionState::SCREEN_DOZE, &ScreenStateMachine::DoSetDisplayState};
    stateMachine_[{ScreenTransitionState::SCREEN_OFF, ScreenPowerEvent::SET_DISPLAY_STATE_DOZE_SUSPEND}] = {
        ScreenTransitionState::SCREEN_DOZE_SUSPEND, &ScreenStateMachine::DoSetDisplayState};
    stateMachine_[{ScreenTransitionState::SCREEN_OFF, ScreenPowerEvent::SUSPEND_BEGIN}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoRecordTransNormal};
    stateMachine_[{ScreenTransitionState::SCREEN_OFF, ScreenPowerEvent::SET_DISPLAY_STATE}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoRecordTransNormal};

    stateMachine_[{ScreenTransitionState::WAIT_SCREEN_ON_READY, ScreenPowerEvent::SET_DISPLAY_STATE}] = {
        ScreenTransitionState::WAIT_SCREEN_ON_READY, &ScreenStateMachine::DoSetDisplayState};
    stateMachine_[{ScreenTransitionState::WAIT_SCREEN_ON_READY, ScreenPowerEvent::POWER_ON}] = {
        ScreenTransitionState::SCREEN_ON, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::WAIT_SCREEN_ON_READY, ScreenPowerEvent::SYNC_POWER_ON}] = {
        ScreenTransitionState::SCREEN_ON, &ScreenStateMachine::DoRecordTransNormal};

    stateMachine_[{ScreenTransitionState::SCREEN_ON, ScreenPowerEvent::WAKEUP_BEGIN}] = {
        ScreenTransitionState::WAIT_SCREEN_ON_READY, &ScreenStateMachine::DoWakeUpBegin};
    stateMachine_[{ScreenTransitionState::SCREEN_ON, ScreenPowerEvent::SUSPEND_BEGIN}] = {
        ScreenTransitionState::SCREEN_ON, &ScreenStateMachine::DoSuspendBegin};
    stateMachine_[{ScreenTransitionState::SCREEN_ON, ScreenPowerEvent::SET_DISPLAY_STATE}] = {
        ScreenTransitionState::WAIT_LOCK_SCREEN_IND, &ScreenStateMachine::DoSetDisplayState};
    stateMachine_[{ScreenTransitionState::SCREEN_ON, ScreenPowerEvent::SET_DISPLAY_STATE_DOZE}] = {
        ScreenTransitionState::WAIT_LOCK_SCREEN_IND, &ScreenStateMachine::DoSetDisplayState};
    stateMachine_[{ScreenTransitionState::SCREEN_ON, ScreenPowerEvent::SET_DISPLAY_STATE_DOZE_SUSPEND}] = {
        ScreenTransitionState::WAIT_LOCK_SCREEN_IND, &ScreenStateMachine::DoSetDisplayState};
    stateMachine_[{ScreenTransitionState::SCREEN_ON, ScreenPowerEvent::POWER_OFF_DIRECTLY}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::ActionScreenPowerOff};
    stateMachine_[{ScreenTransitionState::SCREEN_ON, ScreenPowerEvent::POWER_ON_DIRECTLY}] = {
        ScreenTransitionState::SCREEN_ON, &ScreenStateMachine::DoSetScreenPower};
    
    stateMachine_[{ScreenTransitionState::WAIT_LOCK_SCREEN_IND, ScreenPowerEvent::POWER_OFF}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::WAIT_LOCK_SCREEN_IND, ScreenPowerEvent::E_DOZE}] = {
        ScreenTransitionState::SCREEN_DOZE, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::WAIT_LOCK_SCREEN_IND, ScreenPowerEvent::SUSPEND}] = {
        ScreenTransitionState::WAIT_LOCK_SCREEN_IND, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::WAIT_LOCK_SCREEN_IND, ScreenPowerEvent::DMS_POWER_CB_END}] = {
        ScreenTransitionState::WAIT_SCREEN_CTRL_RSP, &ScreenStateMachine::DoRecordTransNormal,
        SCREEN_STATE_AOD_TIMEOUT_MS, &ScreenStateMachine::DoAodExitAndSetPowerAllOff};

    stateMachine_[{ScreenTransitionState::WAIT_SCREEN_CTRL_RSP, ScreenPowerEvent::POWER_ON_DIRECTLY}] = {
        ScreenTransitionState::SCREEN_ON, &ScreenStateMachine::DoAodExitAndSetPowerOn};
    stateMachine_[{ScreenTransitionState::WAIT_SCREEN_CTRL_RSP, ScreenPowerEvent::AOD_ENTER_FAIL}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoAodExitAndSetPowerOff};
    stateMachine_[{ScreenTransitionState::WAIT_SCREEN_CTRL_RSP, ScreenPowerEvent::POWER_OFF_DIRECTLY}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoAodExitAndSetPowerOff};
    stateMachine_[{ScreenTransitionState::WAIT_SCREEN_CTRL_RSP, ScreenPowerEvent::E_ADVANCED_ON}] = {
        ScreenTransitionState::SCREEN_ADVANCED_ON, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::WAIT_SCREEN_CTRL_RSP, ScreenPowerEvent::AOD_ENTER_SUCCESS}] = {
        ScreenTransitionState::SCREEN_AOD, &ScreenStateMachine::DoRecordTransNormal};
    stateMachine_[{ScreenTransitionState::WAIT_SCREEN_CTRL_RSP, ScreenPowerEvent::WAKEUP_BEGIN}] = {
        ScreenTransitionState::WAIT_SCREEN_ON_READY, &ScreenStateMachine::DoWakeUpBegin};

    stateMachine_[{ScreenTransitionState::SCREEN_AOD, ScreenPowerEvent::WAKEUP_BEGIN}] = {
        ScreenTransitionState::WAIT_SCREEN_ON_READY, &ScreenStateMachine::DoWakeUpBegin};
    stateMachine_[{ScreenTransitionState::SCREEN_AOD, ScreenPowerEvent::E_ADVANCED_ON}] = {
        ScreenTransitionState::SCREEN_ADVANCED_ON, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::SCREEN_AOD, ScreenPowerEvent::POWER_OFF}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::SCREEN_AOD, ScreenPowerEvent::POWER_OFF_DIRECTLY}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoSetScreenPower};

    stateMachine_[{ScreenTransitionState::SCREEN_ADVANCED_ON, ScreenPowerEvent::WAKEUP_BEGIN}] = {
        ScreenTransitionState::WAIT_SCREEN_ADVANCED_ON_READY, &ScreenStateMachine::DoWakeUpBegin};
    stateMachine_[{ScreenTransitionState::SCREEN_ADVANCED_ON, ScreenPowerEvent::WAKEUP_BEGIN_ADVANCED}] = {
        ScreenTransitionState::WAIT_SCREEN_ADVANCED_ON_READY, &ScreenStateMachine::DoWakeUpBegin};
    stateMachine_[{ScreenTransitionState::SCREEN_ADVANCED_ON, ScreenPowerEvent::E_ADVANCED_OFF}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::SCREEN_ADVANCED_ON, ScreenPowerEvent::POWER_OFF_DIRECTLY}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::SCREEN_ADVANCED_ON, ScreenPowerEvent::POWER_ON_DIRECTLY}] = {
        ScreenTransitionState::SCREEN_ON, &ScreenStateMachine::DoScreenPowerOn};
    stateMachine_[{ScreenTransitionState::SCREEN_ADVANCED_ON, ScreenPowerEvent::SUSPEND_BEGIN}] = {
        ScreenTransitionState::SCREEN_ADVANCED_ON, &ScreenStateMachine::DoSuspendBegin};
    stateMachine_[{ScreenTransitionState::SCREEN_ADVANCED_ON, ScreenPowerEvent::SET_DISPLAY_STATE}] = {
        ScreenTransitionState::WAIT_LOCK_SCREEN_IND, &ScreenStateMachine::DoSetDisplayState};

    stateMachine_[{ScreenTransitionState::SCREEN_DOZE, ScreenPowerEvent::WAKEUP_BEGIN}] = {
        ScreenTransitionState::WAIT_SCREEN_ON_READY, &ScreenStateMachine::DoWakeUpBegin,
        SCREEN_STATE_NORMAL_TIMEOUT_MS, &ScreenStateMachine::DoScreenPowerOn};
    stateMachine_[{ScreenTransitionState::SCREEN_DOZE, ScreenPowerEvent::POWER_OFF}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::SCREEN_DOZE, ScreenPowerEvent::E_DOZE_SUSPEND}] = {
        ScreenTransitionState::SCREEN_DOZE_SUSPEND, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::SCREEN_DOZE, ScreenPowerEvent::SET_DISPLAY_STATE}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoSetDisplayState};
    stateMachine_[{ScreenTransitionState::SCREEN_DOZE, ScreenPowerEvent::SET_DISPLAY_STATE_DOZE_SUSPEND}] = {
        ScreenTransitionState::SCREEN_DOZE_SUSPEND, &ScreenStateMachine::DoSetDisplayState};

    stateMachine_[{ScreenTransitionState::SCREEN_DOZE_SUSPEND, ScreenPowerEvent::E_DOZE}] = {
        ScreenTransitionState::SCREEN_DOZE, &ScreenStateMachine::DoSetScreenPower};
    stateMachine_[{ScreenTransitionState::SCREEN_DOZE_SUSPEND, ScreenPowerEvent::WAKEUP_BEGIN}] = {
        ScreenTransitionState::WAIT_SCREEN_ON_READY, &ScreenStateMachine::DoWakeUpBegin,
        SCREEN_STATE_NORMAL_TIMEOUT_MS, &ScreenStateMachine::DoScreenPowerOn};
    stateMachine_[{ScreenTransitionState::SCREEN_DOZE_SUSPEND, ScreenPowerEvent::SET_DISPLAY_STATE}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoSetDisplayState};
    stateMachine_[{ScreenTransitionState::SCREEN_DOZE_SUSPEND, ScreenPowerEvent::SET_DISPLAY_STATE_DOZE}] = {
        ScreenTransitionState::SCREEN_DOZE, &ScreenStateMachine::DoSetDisplayState};

    stateMachine_[{ScreenTransitionState::WAIT_SCREEN_ADVANCED_ON_READY,
        ScreenPowerEvent::SET_SCREEN_POWER_FOR_ALL_POWER_ON}] = {
        ScreenTransitionState::SCREEN_ON, &ScreenStateMachine::DoSetScreenPowerForAll};
    stateMachine_[{ScreenTransitionState::WAIT_SCREEN_ADVANCED_ON_READY,
        ScreenPowerEvent::SET_SCREEN_POWER_FOR_ALL_POWER_OFF}] = {
        ScreenTransitionState::SCREEN_OFF, &ScreenStateMachine::DoSetScreenPowerForAll};
}

} // namespace OHOS::Rosen