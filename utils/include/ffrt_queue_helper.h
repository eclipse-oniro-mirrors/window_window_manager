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

#ifndef OHOS_ROSEN_WINDOW_SCENE_FFRT_QUEUE_HELPER_H
#define OHOS_ROSEN_WINDOW_SCENE_FFRT_QUEUE_HELPER_H

#include <atomic>
#include <cstdint>
#include <memory>

#include "timeout_future.h"

namespace ffrt {
class queue;
} // namespace ffrt

namespace OHOS::Rosen {
class FfrtQueueHelper {
public:
    FfrtQueueHelper();
    ~FfrtQueueHelper();
    bool SubmitTaskAndWait(std::function<void()>&& task, uint64_t timeout);
    void SubmitTask(std::function<void()>&& task);
    void SubmitTaskToHead(std::function<void()>&& task);

    void StartStatCollection();
    void StopStatCollection();

private:
    void StartStatTimer();
    void ParseAndReportStat();
    static void OnStatTimer(void* data);

private:
    std::unique_ptr<ffrt::queue> ffrtQueue_;
    std::unique_ptr<char[]> statBuffer_;
    uint32_t statBufferSize_ = 0;
    int statTimer_ = -1;
    std::atomic<bool> statEnabled_ {false};
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_FFRT_QUEUE_HELPER_H
