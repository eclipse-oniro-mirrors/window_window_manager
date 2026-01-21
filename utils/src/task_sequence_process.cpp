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
    : maxQueueSize_(maxQueueSize), maxTimeInterval_(maxTimeInterval), timerName_(timerName)
{
    maxQueueSize_ = std::max(DEFAULT_QUEUE_SIZE, maxQueueSize);
    if (!CreateSysTimer()) {
        TLOGE(WmsLogTag::DMS, "TaskSequenceProcess created fail, maxQueueSize: %{public}u", maxQueueSize_);
    }
}

TaskSequenceProcess::TaskSequenceProcess(uint32_t maxQueueSize, uint32_t maxQueueNumber, uint64_t maxTimeInterval,
    std::string timerName)
    : maxQueueSize_(maxQueueSize), maxQueueNumber_(maxQueueNumber), maxTimeInterval_(maxTimeInterval),
    timerName_(timerName)
{
    maxQueueSize_ = std::max(DEFAULT_QUEUE_SIZE, maxQueueSize);
    maxQueueNumber_ = std::max(DEFAULT_QUEUE_NUMBER, maxQueueNumber);
    if (!CreateSysTimer()) {
        TLOGE(WmsLogTag::DMS, "TaskSequenceProcess created fail, maxQueueSize: %{public}u", maxQueueSize_);
    }
}

TaskSequenceProcess::~TaskSequenceProcess()
{
    DestroySysTimer();
};

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
    std::lock_guard<std::mutex> lock(taskQueueMapMutex_);
    uint64_t queueId = DEFAULT_QUEUE_ID;
    if (!FindMinSnTaskQueueId(queueId)) {
        TLOGE(WmsLogTag::DMS, "TaskSequenceProcess is empty");
        return nullptr;
    }
    if (taskRunningFlag_.load()) {
        TLOGI(WmsLogTag::DMS, "TaskSequenceProcess do not pop");
        return nullptr;
    }
    std::function<void()> task = taskQueueMap_[queueId].front().task;
    taskRunningFlag_.store(true);
    if (!StartSysTimer()) {
        taskRunningFlag_.store(false);
        TLOGE(WmsLogTag::DMS, "TaskSequenceProcess do not StartSysTimer succ");
    }
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

void TaskSequenceProcess::ExecTask()
{
    std::function<void()> task = PopFromQueue();
    if (!task) {
        TLOGE(WmsLogTag::DMS, "TaskSequenceProcess do not execute");
        return;
    }
    task();
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
    StopSysTimer();
    ExecTask();
}

bool TaskSequenceProcess::CreateSysTimer()
{
    TLOGI(WmsLogTag::DMS, "TaskSequenceProcess CreatSysTimer");

    std::lock_guard<std::mutex> lock(timerMutex_);
    if (taskTimerId_ != 0) {
        TLOGW(WmsLogTag::DMS, "TaskTimerId is not zero");
        return false;
    }
    std::shared_ptr<WindowSysTimer> taskSysTimer = std::make_unique<WindowSysTimer>(false, maxTimeInterval_, false);
    std::function<void()> callback = [this]() {
        taskRunningFlag_.store(false);
        ExecTask();
    };
    taskSysTimer->SetCallbackInfo(callback);
    taskSysTimer->SetName(timerName_);
    taskTimerId_ = MiscServices::TimeServiceClient::GetInstance()->CreateTimer(taskSysTimer);

    return taskTimerId_ ? true : false;
}

void TaskSequenceProcess::DestroySysTimer()
{
    std::lock_guard<std::mutex> lock(timerMutex_);
    MiscServices::TimeServiceClient::GetInstance()->DestroyTimer(taskTimerId_);
    taskTimerId_ = 0;
}

bool TaskSequenceProcess::StartSysTimer()
{
    std::lock_guard<std::mutex> lock(timerMutex_);
    if (taskTimerId_ == 0) {
        TLOGW(WmsLogTag::DMS, "TaskTimerId is zero");
        return false;
    }
    auto currentTime = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
    uint64_t triggerTime = currentTime + maxTimeInterval_;
    bool result = MiscServices::TimeServiceClient::GetInstance()->StartTimer(taskTimerId_, triggerTime);
    return result;
}

void TaskSequenceProcess::StopSysTimer()
{
    std::lock_guard<std::mutex> lock(timerMutex_);
    if (taskTimerId_ == 0) {
        TLOGW(WmsLogTag::DMS, "TaskSequenceProcess TaskTimerId is zero");
        return;
    }
    MiscServices::TimeServiceClient::GetInstance()->StopTimer(taskTimerId_);
}
} // namespace OHOS::Rosen