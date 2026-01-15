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

#ifndef TASK_SEQUENCE_PROCESS_H
#define TASK_SEQUENCE_PROCESS_H
#include <mutex>
#include <queue>
#include <atomic>
#include "window_system_timer.h"

namespace OHOS::Rosen {

class TaskSequenceProcess {
public:
    explicit TaskSequenceProcess(uint32_t maxQueueSize, uint64_t maxTimeInterval, std::string timerName);
    ~TaskSequenceProcess();
    void AddTask(const std::function<void()>& task);
    void FinishTask();

private:
    std::function<void()> PopFromQueue();
    void PushToQueue(const std::function<void()>& task);
    void ExecTask();
    bool CreateSysTimer();
    void DestroySysTimer();
    bool StartSysTimer();
    void StopSysTimer();
    uint32_t maxQueueSize_ {1};
    uint64_t maxTimeInterval_ {1000};
    uint64_t taskTimerId_ {0};
    std::string timerName_;
    std::queue<std::function<void()>> taskQueue_;
    std::mutex queueMutex_;
    std::mutex timerMutex_;
    std::atomic<bool> taskRunningFlag_ {false};
    WindowSysTimer TaskTimer;
};
} //namespace OHOS::Rosen
#endif