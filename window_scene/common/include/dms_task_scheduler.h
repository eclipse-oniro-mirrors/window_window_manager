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
#ifndef OHOS_ROSEN_DMS_TASK_SCHEDULER_H 
#define OHOS_ROSEN_DMS_TASK_SCHEDULER_H

#include "task_scheduler.h"

namespace OHOS::Rosen {
class SafeTaskScheduler : public TaskScheduler {
public:
    explicit SafeTaskScheduler(const std::string& threadName) : TaskScheduler(threadName) {};
    ~SafeTaskScheduler() {};
    void PostAsyncTask(Task&& task, const std::string& name, int64_t delayTime = 0) override;
    void PostVoidSyncTask(Task&& task, const std::string& name) override;
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_DMS_TASK_SCHEDULER_H