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
TaskSequenceProcess::TaskSequenceProcess(uint32_t maxQueueSize, uint64_t maxTimeInterval, std::string timerName)
    : maxQueueSize_(maxQueueSize), maxTimeInterval_(maxTimeInterval), timerName_(timerName)
{
    if (maxQueueSize_ <= 0) {
        maxQueueSize_ = 1;
    }
    if (!CreateSysTimer()) {
        TLOGE(WmsLogTag::DMS, "TaskSequenceProcess created fail, maxQueueSize: %{public}u", maxQueueSize_);
    }
}

TaskSequenceProcess::~TaskSequenceProcess()
{
    DestroySysTimer();
};

std::function<void()> TaskSequenceProcess::PopFromQueue()
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (taskQueue_.empty() || taskRunningFlag_.load()) {
        TLOGI(WmsLogTag::DMS, "TaskSequenceProcess do not pop");
        return nullptr;
    }
    std::function<void()> task = taskQueue_.front();
    taskRunningFlag_.store(true);
    if (!StartSysTimer()) {
        taskRunningFlag_.store(false);
        TLOGE(WmsLogTag::DMS, "TaskSequenceProcess do not StartSysTimer succ");
    }
    taskQueue_.pop();
    return task;
}

void TaskSequenceProcess::PushToQueue(const std::function<void()>& task)
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (taskQueue_.size() >= maxQueueSize_) {
        taskQueue_.pop();
    }
    taskQueue_.push(task);
}

void TaskSequenceProcess::ExecTask()
{
    std::function<void()> task = PopFromQueue();
    if (!task) {
        TLOGD(WmsLogTag::DMS, "TaskSequenceProcess do not execute");
        return;
    }
    task();
}

void TaskSequenceProcess::AddTask(const std::function<void()>& task)
{
    PushToQueue(task);
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
    std::function<void()> callback = [this]() { taskRunningFlag_.store(false); };
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