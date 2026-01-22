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
#include <map>
#include <queue>
#include <atomic>
#include <chrono>
#include <cstdint>

#include "ffrt.h"
#include "ffrt_timer.h"
#include "task_scheduler.h"

namespace OHOS::Rosen {

struct TaskInfo {
    uint64_t sn{0};
    std::function<void()> task{nullptr};
    TaskInfo(uint64_t sn, const std::function<void()>& task): sn(sn), task(task) {}
};

class TaskSequenceProcess {
public:
    explicit TaskSequenceProcess(uint32_t maxQueueSize, uint64_t maxTimeInterval);
    explicit TaskSequenceProcess(uint32_t maxQueueSize, uint32_t maxQueueNumber, uint64_t maxTimeInterval);
    ~TaskSequenceProcess();
    void AddTask(const std::function<void()>& task);
    void AddTask(uint64_t id, const std::function<void()>& task);
    void FinishTask();
    void SetTaskScheduler(std::shared_ptr<TaskScheduler> scheduler);

private:
    bool FindMinSnTaskQueueId(uint64_t& minSnTaskQueueId);
    std::function<void()> PopFromQueue();
    void PushToQueue(uint64_t sn, const TaskInfo& taskInfo);
    bool StartSysTimer();
    void ExecTask();
    void OnTimerTask();
    uint32_t maxQueueSize_ {1};
    uint32_t maxQueueNumber_ {1};
    uint64_t maxTimeInterval_ {1000};
    uint64_t taskTimerId_ {0};
    std::map<uint64_t, std::queue<TaskInfo>> taskQueueMap_;
    std::mutex taskQueueMapMutex_;
    std::mutex timerMutex_;
    std::atomic<bool> taskRunningFlag_ {false};
    std::atomic<uint64_t> sn_{0};
    uint64_t currentTimeMs_ {0};
    std::unique_ptr<FfrtTimer> cacheTimer_ = nullptr;
    std::shared_ptr<TaskScheduler> taskScheduler_ = nullptr;
};
} //namespace OHOS::Rosen
#endif