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

#include "ffrt_queue.h"

#include <vector>
#include "ffrt_inner.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace DMS {
constexpr int32_t FFRT_USER_INTERACTIVE_MAX_THREAD_NUM = 5;

FfrtQueue::FfrtQueue(const std::string& queueName)
    : FfrtQueue(queueName, FfrtQueueOptions{}) {}

FfrtQueue::FfrtQueue(const std::string& queueName, const FfrtQueueOptions& queueOptions)
    : name_(queueName), mode_(queueOptions.mode)
{
    InitFfrtQueue(queueName, queueOptions);
}

void FfrtQueue::InitFfrtQueue(const std::string& queueName, const FfrtQueueOptions& queueOptions)
{
    if (queueOptions.mode == FfrtQueueMode::SERIAL) {
        queue_ = std::make_unique<ffrt::queue>(queueName.c_str(), ffrt::queue_attr().qos(ffrt_qos_user_interactive));
        TLOGI(WmsLogTag::DMS, "Ffrt created SERIAL queue: %{public}s", queueName.c_str());
    } else {
        int32_t concurrency = queueOptions.maxConcurrency;
        if (concurrency <= 0) {
            concurrency = FFRT_USER_INTERACTIVE_MAX_THREAD_NUM;
        }
        
        queue_ = std::make_unique<ffrt::queue>(
            ffrt::queue_concurrent,
            queueName.c_str(),
            ffrt::queue_attr()
                .max_concurrency(concurrency)
                .qos(ffrt_qos_user_interactive)
        );

        TLOGI(WmsLogTag::DMS, "Ffrt created CONCURRENT queue: %{public}s, concurrency=%{public}d",
            queueName.c_str(), concurrency);
    }

    if (queue_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Ffrt create failed %{public}s", name_.c_str());
    }
}

FfrtQueue::~FfrtQueue()
{
    TLOGI(WmsLogTag::DMS, "Ffrt destory %{public}s", name_.c_str());
    if (queue_ == nullptr) {
        return;
    }
    queue_.reset();
}

void FfrtQueue::Reset()
{
    TLOGI(WmsLogTag::DMS, "Reset ffrt %{public}s", name_.c_str());
    if (queue_ == nullptr) {
        return;
    }
    queue_.reset();
}

DMError FfrtQueue::Submit(const std::function<void()>& func)
{
    if (queue_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Invalid queue %{public}s", name_.c_str());
        return DMError::DM_ERROR_INVALID_PARAM;
    }

    queue_->submit(func);
    return DMError::DM_OK;
}

DMError FfrtQueue::Submit(const std::function<void()>&& func)
{
    if (queue_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Invalid queue %{public}s", name_.c_str());
        return DMError::DM_ERROR_INVALID_PARAM;
    }

    queue_->submit(std::move(func));
    return DMError::DM_OK;
}

DMError FfrtQueue::Submit(const std::function<void()>& func, const std::string& taskName, const int64_t& delayTime)
{
    if (queue_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Invalid queue %{public}s %{public}s", name_.c_str(), taskName.c_str());
        return DMError::DM_ERROR_INVALID_PARAM;
    }

    queue_->submit(func, ffrt::task_attr().name(taskName.c_str()).delay(delayTime));
    return DMError::DM_OK;
}

DMError FfrtQueue::Submit(const std::function<void()>&& func, const std::string& taskName, const int64_t& delayTime)
{
    if (queue_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Invalid queue %{public}s %{public}s", name_.c_str(), taskName.c_str());
        return DMError::DM_ERROR_INVALID_PARAM;
    }

    queue_->submit(std::move(func), ffrt::task_attr().name(taskName.c_str()).delay(delayTime));
    return DMError::DM_OK;
}

DMError FfrtQueue::SyncSubmit(std::function<void()>& func)
{
    if (queue_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Invalid queue %{public}s", name_.c_str());
        return DMError::DM_ERROR_INVALID_PARAM;
    }

    ffrt::task_handle handler = queue_->submit_h(func);
    queue_->wait(handler);
    return DMError::DM_OK;
}

DMError FfrtQueue::SyncSubmit(std::function<void()>&& func)
{
    if (queue_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Invalid queue %{public}s", name_.c_str());
        return DMError::DM_ERROR_INVALID_PARAM;
    }

    ffrt::task_handle handler = queue_->submit_h(std::move(func));
    queue_->wait(handler);
    return DMError::DM_OK;
}

} // namespace OHOS
} // namespace ROSEN
} // namespace DMS
