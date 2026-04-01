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

#ifndef DMS_FFRT_QUEUE_H
#define DMS_FFRT_QUEUE_H

#include "ffrt.h"
#include "dm_common.h"

namespace OHOS {
namespace Rosen {
namespace DMS {
using Rosen::DMError;

class FfrtQueue {
public:

    /**
     * @brief ffrt queue mode
     */
    enum class FfrtQueueMode {
        SERIAL,
        CONCURRENT
    };

    /**
     * @brief ffrt queue options
     */
    struct FfrtQueueOptions {
        FfrtQueueMode mode = FfrtQueueMode::SERIAL;
        int32_t maxConcurrency = -1;
    };

    FfrtQueue() = default;
    FfrtQueue(const std::string& queueName);
    FfrtQueue(const std::string& queueName, const FfrtQueueOptions& queueOptions);

    ~FfrtQueue();

    /**
     * @brief reset this queue.
     */
    void Reset();

    /**
     * @brief Submits a task to this queue.
     */
    DMError Submit(const std::function<void()>& func);

    /**
     * @brief Submits a task to this queue.
     */
    DMError Submit(const std::function<void()>&& func);

    /**
     * @brief Submits a task with a specified attribute to this queue.
     */
    DMError Submit(const std::function<void()>& func, const std::string& taskName, const int64_t &delayTime);

    /**
     * @brief Submits a task with a specified attribute to this queue.
     */
    DMError Submit(const std::function<void()>&& func, const std::string& taskName, const int64_t& delayTime);

    /**
     * @brief Submits a sync task to this queue.
     */
    DMError SyncSubmit(std::function<void()>& func);

    /**
     * @brief Submits a sync task to this queue.
     */
    DMError SyncSubmit(std::function<void()>&& func);

private:
    void InitFfrtQueue(const std::string& queueName, const FfrtQueueOptions& queueOptions);
    std::string name_;
    FfrtQueueMode mode_ = FfrtQueueMode::SERIAL;
    std::unique_ptr<ffrt::queue> queue_ = nullptr;
};
} // namespace OHOS
} // namespace ROSEN
} // namespace DMS
#endif  // DMS_FFRT_QUEUE_H
