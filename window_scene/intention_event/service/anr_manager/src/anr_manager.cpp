/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "anr_manager.h"

#include <algorithm>
#include <vector>

#include "dfx_hisysevent.h"
#include "entrance_log.h"
#include "proto.h"
#include "timer_manager.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "ANRManager" };
constexpr int32_t MAX_ANR_TIMER_COUNT { 64 };
} // namespace

ANRManager::ANRManager() {}
ANRManager::~ANRManager() {}

void ANRManager::Init()
{
    CALL_DEBUG_ENTER;
    SwitchAnr(true);
    TimerMgr->Init();
}

void ANRManager::AddTimer(int32_t eventId, int32_t persistentId)
{
    std::lock_guard<std::mutex> guard(mtx_);
    if (!switcher_) {
        WLOGFD("Anr is off, dispatch event without timer");
        return;
    }
    if (anrTimerCount_ >= MAX_ANR_TIMER_COUNT) {
        WLOGFD("AddAnrTimer failed, anrTimerCount exceeded %{public}d", MAX_ANR_TIMER_COUNT);
        return;
    }
    int32_t timerId = TimerMgr->AddTimer(ANRTimeOutTime::INPUT_UI_TIMEOUT_TIME, [this, eventId, persistentId]() {
        WLOGFD("Anr callback enter. persistentId:%{public}d, eventId:%{public}d", persistentId, eventId);
        eventStage_.SetAnrStatus(persistentId, true);
        AppInfo appInfo = GetAppInfoByPersistentId(persistentId);
        DfxHisysevent::ApplicationBlockInput(eventId, appInfo.pid, appInfo.processName, persistentId);
        WLOGFE("Application not responding. persistentId:%{public}d, eventId:%{public}d,"
            "pid:%{public}d, processName:%{public}s",
            persistentId, eventId, appInfo.pid, appInfo.processName.c_str());
        if (anrObserver_ != nullptr) {
            anrObserver_(appInfo.pid);
        } else {
            WLOGFE("AnrObserver is nullptr, do nothing");
        }
        std::vector<int32_t> timerIds = eventStage_.GetTimerIds(persistentId);
        for (int32_t item : timerIds) {
            if (item != -1) {
                TimerMgr->RemoveTimer(item);
                anrTimerCount_--;
            }
        }
        WLOGFD("Anr callback leave. persistentId:%{public}d, eventId:%{public}d", persistentId, eventId);
    });
    anrTimerCount_++;
    eventStage_.SaveANREvent(persistentId, eventId, timerId);
}

void ANRManager::MarkProcessed(int32_t eventId, int32_t persistentId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    WLOGFD("Event: eventId:%{public}d, persistentId:%{public}d", eventId, persistentId);
    std::list<int32_t> timerIds = eventStage_.DelEvents(persistentId, eventId);
    for (int32_t item : timerIds) {
        if (item != -1) {
            TimerMgr->RemoveTimer(item);
            anrTimerCount_--;
        }
    }
}

bool ANRManager::IsANRTriggered(int32_t persistentId)
{
    std::lock_guard<std::mutex> guard(mtx_);
    if (eventStage_.CheckAnrStatus(persistentId)) {
        WLOGFD("Application not respond, persistentId:%{public}d -> applicationPid:%{public}d, processName:%{public}s",
            persistentId, applicationMap_[persistentId].pid, applicationMap_[persistentId].processName.c_str());
        return true;
    }
    return false;
}

void ANRManager::OnSessionLost(int32_t persistentId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    WLOGFD("Disconnect session, persistentId:%{public}d -> applicationPid:%{public}d, processName:%{public}s",
        persistentId, applicationMap_[persistentId].pid, applicationMap_[persistentId].processName.c_str());
    RemoveTimers(persistentId);
    RemovePersistentId(persistentId);
}

void ANRManager::SetApplicationInfo(int32_t persistentId, int32_t applicationPid, std::string processName)
{
    std::lock_guard<std::mutex> guard(mtx_);
    WLOGFD("PersistentId:%{public}d -> applicationPid:%{public}d, processName:%{public}s",
        persistentId, applicationPid, processName.c_str());
    applicationMap_[persistentId] = { applicationPid, processName };
}

void ANRManager::SetAnrObserver(std::function<void(int32_t)> anrObserver)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    anrObserver_ = anrObserver;
}

ANRManager::AppInfo ANRManager::GetAppInfoByPersistentId(int32_t persistentId)
{
    if (applicationMap_.find(persistentId) != applicationMap_.end()) {
        WLOGFD("PersistentId:%{public}d -> pid:%{public}d, processName:%{public}s",
            persistentId, applicationMap_[persistentId].pid, applicationMap_[persistentId].processName.c_str());
        return applicationMap_[persistentId];
    }
    WLOGFD("No application matches persistentId:%{public}d", persistentId);
    return ANRManager::AppInfo();
}

void ANRManager::RemoveTimers(int32_t persistentId)
{
    std::vector<int32_t> timerIds = eventStage_.GetTimerIds(persistentId);
    for (int32_t item : timerIds) {
        if (item != -1) {
            TimerMgr->RemoveTimer(item);
            anrTimerCount_--;
        }
    }
}

void ANRManager::RemovePersistentId(int32_t persistentId)
{
    applicationMap_.erase(persistentId);
    eventStage_.OnSessionLost(persistentId);
}

void ANRManager::SwitchAnr(bool status)
{
    switcher_ = status;
    if (switcher_) {
        WLOGFI("Anr is on");
    } else {
        WLOGFI("Anr is off");
    }
}
} // namespace Rosen
} // namespace OHOS
