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

#ifndef SCREEN_STATE_TIMER_H
#define SCREEN_STATE_TIMER_H

#include <mutex>
#include <map>
#include <future>

#include "common/include/task_scheduler.h"
#include "dms_global_mutex.h"

namespace OHOS::Rosen {
class ScreenStateTimer : public RefBase {
public:
    ScreenStateTimer() = default;
    ~ScreenStateTimer() = default;
    ScreenStateTimer(const ScreenStateTimer&) = delete;
    ScreenStateTimer& operator=(const ScreenStateTimer&) = delete;
    void StartTimer(int32_t state, uint32_t delayMs, std::function<void()> task);
    void StopTimer(int32_t state);

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> is_running_;
    std::map<int32_t, std::function<void()>> stateCancelMap_;
};

} // namespace OHOS::Rosen
#endif // SCREEN_STATE_TIMER_H