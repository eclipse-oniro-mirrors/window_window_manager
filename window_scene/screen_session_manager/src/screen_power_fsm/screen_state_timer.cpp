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
#include "screen_session_manager.h"
#include "dms_global_mutex.h"

namespace OHOS::Rosen {
void ScreenStateTimer::StartTimer(int32_t state, uint32_t delayMs, TaskScheduler::Task task)
{
    if (delayMs <= 0) {
        return;
    }

    auto ffrtQueueHelper = ScreenSessionManager::GetInstance().GetFfrtQueueHelper();
    stateCancelMap_[state] = task;
    auto expectTime = std::chrono::steady_clock::now();
    if (ffrtQueueHelper == nullptr) {
        TLOGNFI(WmsLogTag::DMS, "[ScreenPower FSM] ffrtQueueHelper is nullptr");
        return;
    }
    ffrtQueueHelper->SubmitTask([this, state, delayMs, expectTime] {
        auto currentTime = std::chrono::steady_clock::now();
        std::unique_lock<std::mutex> lock(mutex_);
        auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - expectTime).count();
        auto delayTmp = delayMs - interval;
        TLOGNFI(WmsLogTag::DMS, "[ScreenPower FSM] task start delay time: %{public}lld", delayTmp);
        if (delayTmp <= 0 || std::cv_status::timeout == DmUtils::safe_wait_for(cv_, lock,
            std::chrono::milliseconds(delayTmp))) {
            if (stateCancelMap_.find(state) != stateCancelMap_.end()) {
                stateCancelMap_[state]();
            }
        }
        stateCancelMap_.erase(state);
    });
}

void ScreenStateTimer::StopTimer(int32_t state)
{
    std::unique_lock<std::mutex> lock(mutex_);
    TLOGNFI(WmsLogTag::DMS, "[ScreenPower FSM] current state: %{public}d", state);
    stateCancelMap_.erase(state);
    cv_.notify_all();
}

} // namespace OHOS::Rosen