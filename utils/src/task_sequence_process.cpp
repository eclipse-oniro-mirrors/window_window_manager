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

#include "ffrt_timer.h"
#include "task_sequence_process.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
constexpr uint64_t INVALID_TASK_SN = UINT64_MAX;
constexpr uint32_t INVALID_QUEUE_SIZE = 0;
constexpr uint32_t INVALID_QUEUE_NUMBER = 0;
constexpr uint64_t DEFAULT_QUEUE_ID = 0;
constexpr uint64_t DEFAULT_TASK_SN = 0;
constexpr uint32_t DEFAULT_QUEUE_SIZE = 1;
constexpr uint32_t DEFAULT_QUEUE_NUMBER = 1;

TaskSequenceProcess::TaskSequenceProcess(uint32_t maxQueueSize, uint64_t maxTimeInterval, std::string timerName)
    : maxQueueSize_(maxQueueSize), maxTimeInterval_(maxTimeInterval)
{
    maxQueueSize_ = std::max(DEFAULT_QUEUE_SIZE, maxQueueSize);
}

TaskSequenceProcess::TaskSequenceProcess(uint32_t maxQueueSize, uint32_t maxQueueNumber, uint64_t maxTimeInterval,
    std::string timerName)
    : maxQueueSize_(maxQueueSize), maxQueueNumber_(maxQueueNumber), maxTimeInterval_(maxTimeInterval)
{
    maxQueueSize_ = std::max(DEFAULT_QUEUE_SIZE, maxQueueSize);
    maxQueueNumber_ = std::max(DEFAULT_QUEUE_NUMBER, maxQueueNumber);
}

TaskSequenceProcess::~TaskSequenceProcess() = default;

bool TaskSequenceProcess::FindMinSnTaskQueueId(uint64_t& minSnTaskQueueId)
{
    if (taskQueueMap_.empty()) {
        return false;
    }
    auto pair = taskQueueMap_.begin();
    minSnTaskQueueId = pair->first;
    uint64_t minSn = INVALID_TASK_SN;
    for (auto it = taskQueueMap_.begin(); it != taskQueueMap_.end(); ++it) {
        if (it->second.empty()) {
            continue;
        }
        uint64_t currentSn = it->second.front().sn;
        if (currentSn < minSn) {
            minSn = currentSn;
            minSnTaskQueueId = it->first;
        }
    }
    return minSn == INVALID_TASK_SN ? false : true;
}

std::function<void()> TaskSequenceProcess::PopFromQueue()
{
    auto now = std::chrono::system_clock::now();
    uint64_t startTimeMs = std::chrono::time_point_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::lock_guard<std::mutex> lock(taskQueueMapMutex_);
    if (taskRunningFlag_.load() && startTimeMs > currentTimeMs_ + maxTimeInterval_) {
        taskRunningFlag_.store(false);
        if (cacheTimer_ != nullptr) {
            cacheTimer_->StopTimer();
        }
        TLOGI(WmsLogTag::DMS, "task time out");
    }
    if (taskRunningFlag_.load()) {
        TLOGI(WmsLogTag::DMS, "task flag is true");
        return nullptr;
    }
    uint64_t queueId = DEFAULT_QUEUE_ID;
    if (!FindMinSnTaskQueueId(queueId)) {
        TLOGE(WmsLogTag::DMS, "TaskSequenceProcess is empty");
        return nullptr;
    }
    std::function<void()> task = taskQueueMap_[queueId].front().task;
    taskQueueMap_[queueId].pop();
    return task;
}

void TaskSequenceProcess::PushToQueue(uint64_t id, const TaskInfo& taskInfo)
{
    std::lock_guard<std::mutex> lock(taskQueueMapMutex_);
    if (taskQueueMap_.size() >= maxQueueNumber_) {
        for (auto it = taskQueueMap_.begin(); it != taskQueueMap_.end();) {
            it = it->second.empty() ? taskQueueMap_.erase(it) : ++it;
        }
    }
    if (taskQueueMap_.find(id) == taskQueueMap_.end() && taskQueueMap_.size() >= maxQueueNumber_) {
        TLOGW(WmsLogTag::DMS, "Task push fail, maxQueueNumber: %{public}u", maxQueueNumber_);
        return;
    }
    if (taskQueueMap_[id].size() >= maxQueueSize_) {
        taskQueueMap_[id].pop();
    }
    taskQueueMap_[id].push(taskInfo);
}



void TaskSequenceProcess::AddTask(const std::function<void()>& task)
{
    PushToQueue(DEFAULT_QUEUE_ID, {DEFAULT_TASK_SN, task});
    ExecTask();
}

void TaskSequenceProcess::AddTask(uint64_t id, const std::function<void()>& task)
{
    uint64_t currentSn = sn_.fetch_add(1);
    currentSn = (currentSn + 1) % INVALID_TASK_SN;
    PushToQueue(id, {currentSn, task});
    ExecTask();
}

void TaskSequenceProcess::FinishTask()
{
    taskRunningFlag_.store(false);
    if (cacheTimer_ != nullptr) {
        cacheTimer_->StopTimer();
    }
    TLOGW(WmsLogTag::DMS, "finish task");
    ExecTask();
}

void TaskSequenceProcess::ExecTask()
{
    std::function<void()> task = PopFromQueue();
    auto now = std::chrono::system_clock::now();
    uint64_t currentTimeMs_ = std::chrono::time_point_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    if (!task) {
        TLOGE(WmsLogTag::DMS, "task do not execute");
        return;
    }
    taskRunningFlag_.store(true);
    StartSysTimer();
    TLOGD(WmsLogTag::DMS, "execute task");
    task();
}

bool TaskSequenceProcess::StartSysTimer()
{
    if (taskScheduler_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "taskScheduler is nullptr");
        return false;
    }
    std::lock_guard<std::mutex> lock(timerMutex_);
    cacheTimer_ = std::make_unique<FfrtTimer>();
    cacheTimer_->StartOneShotTimer(maxTimeInterval_, this, [](void *taskSequencePtr) -> void {
        if (taskSequencePtr != nullptr) {
            TaskSequenceProcess *taskSequenceProcess = reinterpret_cast<TaskSequenceProcess*>(&taskSequenceProcess);
            taskSequenceProcess->OnTimerTask();
            TLOGW(WmsLogTag::DMS, "startTimer time out");
        } else {
            TLOGW(WmsLogTag::DMS, "startTimer failed");
        }
    });
}

void TaskSequenceProcess::OnTimerTask()
{
    auto task = [=] {
        taskRunningFlag_.store(false);
        ExecTask();
    };
    taskScheduler_->PostAsyncTask(task, "OnTimerTask");
}

void TaskSequenceProcess::SetTaskScheduler(std::shared_ptr<TaskScheduler> scheduler)
{
    if (scheduler == nullptr) {
        TLOGW(WmsLogTag::DMS, "invalid scheduler pointer");
        return;
    }
    taskScheduler_ = scheduler;
}
} // namespace OHOS::Rosen