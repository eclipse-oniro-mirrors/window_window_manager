/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "screen_session_manager/include/screen_session_manager.h"
#include "window_manager_hilog.h"
#include "session_permission.h"
#include "screen_session_manager_adapter.h"
#include "ipc_skeleton.h"
#include "sys_cap_util.h"
#include "dms_xcollie.h"

namespace OHOS::Rosen {
bool ScreenSessionManager::WakeUpBegin(DisplayId displayId, PowerStateChangeReason reason)
{
    // only power use for concurrent user
    TLOGNFI(WmsLogTag::DMS, "[UL_POWER_IVI]ScreenId: %{public}" PRIu64 ", Reason: %{public}u",
        displayId, static_cast<uint32_t>(reason));
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGNFE(WmsLogTag::DMS, "Permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    std::vector<ScreenId> screenIds = GetAllScreenIds();
    auto iter = std::find(screenIds.begin(), screenIds.end(), displayId);
    if (iter == screenIds.end()) {
        TLOGNFE(WmsLogTag::DMS, "[UL_POWER_IVI] Invalid displayId");
        return false;
    }
    return DoWakeUpBegin(displayId, reason);
}
 
bool ScreenSessionManager::DoWakeUpBegin(DisplayId displayId, PowerStateChangeReason reason)
{
    TLOGNFI(WmsLogTag::DMS, "Start");
    bool notifyResult = ScreenSessionManagerAdapter::GetInstance().
        NotifySpecifiedDisplayPowerEvent(displayId, DisplayPowerEvent::WAKE_UP, EventStatus::BEGIN);
    if (!notifyResult) {
        TLOGNFI(WmsLogTag::DMS, "[UL_POWER_IVI] Agents failed to notify");
        return false;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(displayId);
    if (screenSession == nullptr) {
        TLOGNFE(WmsLogTag::DMS, "[UL_POWER_IVI] Cannot get ScreenSession, screenId: %{public}" PRIu64"", displayId);
        return false;
    }
    screenSession->PowerStatusChange(DisplayPowerEvent::WAKE_UP, EventStatus::BEGIN, reason);
    TLOGNFI(WmsLogTag::DMS, "End");
    return true;
}
 
bool ScreenSessionManager::WakeUpEnd(DisplayId displayId)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGNFE(WmsLogTag::DMS, "Permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    TLOGNFI(WmsLogTag::DMS, "[UL_POWER_IVI] WakeUpEnd enter");
    std::vector<ScreenId> screenIds = GetAllScreenIds();
    auto iter = std::find(screenIds.begin(), screenIds.end(), displayId);
    if (iter == screenIds.end()) {
        TLOGNFE(WmsLogTag::DMS, "[UL_POWER_IVI] Invalid displayId");
        return false;
    }
    return NotifySpecifiedDisplayPowerEvent(displayId, DisplayPowerEvent::WAKE_UP, EventStatus::END,
        PowerStateChangeReason::STATE_CHANGE_REASON_INIT);
}
 
bool ScreenSessionManager::SuspendBegin(DisplayId displayId, PowerStateChangeReason reason)
{
    // only power use for concurrent user
    TLOGNFI(WmsLogTag::DMS, "[UL_POWER_IVI]ScreenId: %{public}" PRIu64 ", Reason: %{public}u",
        displayId, static_cast<uint32_t>(reason));
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGNFE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    std::vector<ScreenId> screenIds = GetAllScreenIds();
    auto iter = std::find(screenIds.begin(), screenIds.end(), displayId);
    if (iter == screenIds.end()) {
        TLOGNFE(WmsLogTag::DMS, "[UL_POWER_IVI]invalid displayId");
        return false;
    }
    return DoSuspendBegin(displayId, reason);
}
 
bool ScreenSessionManager::DoSuspendBegin(DisplayId displayId, PowerStateChangeReason reason)
{
    TLOGNFI(WmsLogTag::DMS, "Start");
    bool notifyResult = ScreenSessionManagerAdapter::GetInstance().
        NotifySpecifiedDisplayPowerEvent(displayId, DisplayPowerEvent::SLEEP, EventStatus::BEGIN);
    if (!notifyResult) {
        TLOGNFI(WmsLogTag::DMS, "[UL_POWER_IVI] Agents failed to notify");
        return false;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(displayId);
    if (screenSession == nullptr) {
        TLOGNFE(WmsLogTag::DMS, "[UL_POWER_IVI]Cannot get ScreenSession, screenId: %{public}" PRIu64"", displayId);
        return false;
    }
    screenSession->PowerStatusChange(DisplayPowerEvent::SLEEP, EventStatus::BEGIN, reason);
    TLOGNFI(WmsLogTag::DMS, "End");
    return true;
}
 
bool ScreenSessionManager::SuspendEnd(DisplayId displayId)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGNFE(WmsLogTag::DMS, "Permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    TLOGNFI(WmsLogTag::DMS, "[UL_POWER_IVI]SuspendEnd enter");
    std::vector<ScreenId> screenIds = GetAllScreenIds();
    auto iter = std::find(screenIds.begin(), screenIds.end(), displayId);
    if (iter == screenIds.end()) {
        TLOGNFE(WmsLogTag::DMS, "[UL_POWER_IVI]invalid displayId");
        return false;
    }
    return NotifySpecifiedDisplayPowerEvent(displayId, DisplayPowerEvent::SLEEP, EventStatus::END,
        PowerStateChangeReason::STATE_CHANGE_REASON_INIT);
}

bool ScreenSessionManager::NotifySpecifiedDisplayPowerEvent(DisplayId displayId, DisplayPowerEvent event,
    EventStatus status, PowerStateChangeReason reason)
{
    bool notifyResult = ScreenSessionManagerAdapter::GetInstance().NotifySpecifiedDisplayPowerEvent(
        displayId, event, status);
    if (!notifyResult) {
        TLOGNFI(WmsLogTag::DMS, "[UL_POWER_IVI] Agents failed to notify");
        return false;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(displayId);
    if (screenSession == nullptr) {
        TLOGNFE(WmsLogTag::DMS, "[UL_POWER_IVI]Cannot get ScreenSession, screenId: %{public}" PRIu64"", displayId);
        return false;
    }
    screenSession->PowerStatusChange(event, status, reason);
    return true;
}

bool ScreenSessionManager::SetDisplayState(DisplayId displayId, DisplayState state)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGNFE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    std::vector<ScreenId> screenIds = GetAllScreenIds();
    auto iter = std::find(screenIds.begin(), screenIds.end(), displayId);
    if (iter == screenIds.end()) {
        TLOGNFE(WmsLogTag::DMS, "[UL_POWER_IVI]invalid displayId");
        return false;
    }
    TLOGNFI(WmsLogTag::DMS, "[UL_POWER_IVI] state: %{public}d", state);
    return DoSetDisplayState(displayId, state);
}
 
bool ScreenSessionManager::DoSetDisplayState(DisplayId displayId, DisplayState state)
{
    sptr<ScreenSession> screenSession = GetScreenSession(displayId);
    if (screenSession == nullptr) {
        TLOGNFW(WmsLogTag::DMS, "[UL_POWER_IVI] screenId: %{public}" PRIu64, displayId);
        return false;
    }
    screenSession->UpdateDisplayState(state);
    return ScreenSessionManagerAdapter::GetInstance().NotifyDisplayStateChangedById(displayId, state);
}

bool ScreenSessionManager::SetScreenPowerForSpecifiedId(DisplayId displayId, ScreenPowerState state,
    PowerStateChangeReason reason)
{
    TLOGNFI(WmsLogTag::DMS, "[UL_POWER_IVI] Set screen power: %{public}d", state);
        if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGNFE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    TLOGNFI(WmsLogTag::DMS, "screen id:%{public}" PRIu64 ", state:%{public}u",
        displayId, state);
    ScreenPowerStatus status;
    switch (state) {
        case ScreenPowerState::POWER_ON: {
            status = ScreenPowerStatus::POWER_STATUS_ON;
            break;
        }
        case ScreenPowerState::POWER_OFF: {
            status = ScreenPowerStatus::POWER_STATUS_OFF;
            break;
        }
        default: {
            TLOGNFW(WmsLogTag::DMS, "[UL_POWER_IVI]SetScreenPowerStatus state not support");
            return false;
        }
    }
    CallRsSetScreenPowerStatusForConcurrent(displayId, status);
    return NotifySpecifiedDisplayPowerEvent(displayId,
        state == ScreenPowerState::POWER_ON ? DisplayPowerEvent::DISPLAY_ON :DisplayPowerEvent::DISPLAY_OFF,
        EventStatus::END, reason);
}

void ScreenSessionManager::CallRsSetScreenPowerStatusForConcurrent(DisplayId displayId, ScreenPowerStatus status)
{
    auto rsSetScreenPowerStatusTask = [=] {
        TLOGNFI(WmsLogTag::DMS, "[UL_POWER_IVI] rsSet Screen Power Status Task start");
        auto screenSession = GetScreenSession(displayId);
        if (screenSession == nullptr) {
            return;
        }
        SetRSScreenPowerStatusExt(displayId, status);
        TLOGNFI(WmsLogTag::DMS, "[UL_POWER_IVI] rsSet Screen Power Status Task end");
    };
    if (ffrtQueueHelper_ == nullptr) {
        screenPowerTaskScheduler_->PostVoidSyncTask(rsSetScreenPowerStatusTask,
            "rsInterface_.SetScreenPowerStatus task");
        TLOGNFI(WmsLogTag::DMS, "[UL_POWER_IVI]PostVoidSyncTask end");
        return;
    }
    constexpr uint64_t SET_SCREEN_POWER_TIMEOUT_MS = 10000;
    bool isTimeOut = ffrtQueueHelper_->SubmitTaskAndWait(std::move(rsSetScreenPowerStatusTask),
        SET_SCREEN_POWER_TIMEOUT_MS);
    if (isTimeOut) {
        TLOGNFE(WmsLogTag::DMS, "[UL_POWER_IVI]CallRsSetScreenPowerStatus timeout, ScreenId: %{public}" PRIu64"",
            displayId);
        return;
    }
    TLOGNFI(WmsLogTag::DMS, "[UL_POWER_IVI]Call Rs Set Screen Power StatusSync end");
}
}
