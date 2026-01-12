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
TaskSequenceProcess::TaskSequenceProcess(uint32_t maxQueueSize)
    : maxQueueSize_(maxQueueSize)
    {
        if (maxQueueSize_ <= 0) {
            maxQueueSize_ = 1;
        }
        TLOGI(WmsLogTag::DMS, "TaskSequenceProcess created with maxQueueSize: %{public}u", maxQueueSize_);
    }
TaskSequenceProcess::~TaskSequenceProcess() = default;

std::function<void()> TaskSequenceProcess::PopFromQueue()
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    TLOGI(WmsLogTag::DMS, "TaskSequenceProcess taskQueue_.size(): %{public}zu", taskQueue_.size());
    if (taskQueue_.empty()) {
        TLOGI(WmsLogTag::DMS, "TaskSequenceProcess do not pop");
        return std::function<void()>();
    }
    std::function<void()> task = taskQueue_.front();
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
        TLOGI(WmsLogTag::DMS, "TaskSequenceProcess do not execute");
        return;
    }
    taskRunningFlag_.store(true);
    task();
}

void TaskSequenceProcess::AddTask(const std::function<void()>& task)
{
    PushToQueue(task);
    ExecTask();
}

void TaskSequenceProcess::FinishTask()
{
    ExecTask();
    taskRunningFlag_.store(false);
}
} // namespace OHOS::Rosen