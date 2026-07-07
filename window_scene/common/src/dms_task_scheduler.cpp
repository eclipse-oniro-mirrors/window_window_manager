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


#include <mutex>
#include <condition_variable>
#include "dms_global_mutex.h"
#include <event_handler.h>
#include <unistd.h>
#include "hitrace_meter.h"
#include "window_manager_hilog.h"
#include "dms_task_scheduler.h"

namespace OHOS {
namespace Rosen {
void SafeTaskScheduler::PostAsyncTask(Task&& task, const std::string& name, int64_t delayTime)
{
    if (delayTime == 0 && handler_->GetEventRunner()->IsCurrentRunnerThread()) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:%s", name.c_str());
        task();
        return;
    }
    auto localTask = [this, task = std::move(task), name] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:%s", name.c_str());
        DmUtils::HoldLock task_lock;
        task();
        ExecuteExportTask();
    };
    handler_->PostTask(std::move(localTask), "wms:" + name, delayTime, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void SafeTaskScheduler::PostVoidSyncTask(Task&& task, const std::string& name)
{
    if (handler_->GetEventRunner()->IsCurrentRunnerThread()) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:%s", name.c_str());
        task();
        return;
    }
    auto localTask = [this, &task, &name] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:%s", name.c_str());
        DmUtils::HoldLock task_lock;
        task();
        ExecuteExportTask();
    };

    DmUtils::DropLock lock;
    handler_->PostSyncTask(std::move(localTask), "wms:" + name, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

} // namespace Rosen
} // namespace OHOS