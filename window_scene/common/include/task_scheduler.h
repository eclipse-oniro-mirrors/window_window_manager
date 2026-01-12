/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_WINDOW_SCENE_TASK_SCHEDULER_H
#define OHOS_ROSEN_WINDOW_SCENE_TASK_SCHEDULER_H

#include <event_handler.h>

#include <unistd.h>
#include "window_manager_hilog.h"

namespace OHOS::Rosen {

void StartTraceForSyncTask(const std::string& name);
void FinishTraceForSyncTask();

class TaskScheduler {
public:
    explicit TaskScheduler(const std::string& threadName);
    virtual ~TaskScheduler() = default;

    std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler();

    using Task = std::function<void()>;
    virtual void PostAsyncTaskToExportHandler(Task&& task, const std::string& name, int64_t delayTime = 0);
    virtual void PostAsyncTask(Task&& task, const std::string& name, int64_t delayTime = 0);
    void PostTask(Task&& task, const std::string& name, int64_t delayTime = 0);
    void RemoveTask(const std::string& name);
    virtual void PostVoidSyncTask(Task&& task, const std::string& name = "ssmTask");
    template<typename SyncTask, typename Return = std::invoke_result_t<SyncTask>>
    Return PostSyncTask(SyncTask&& task, const std::string& name = "ssmTask")
    {
        Return ret;
        if (handler_->GetEventRunner()->IsCurrentRunnerThread()) {
            StartTraceForSyncTask(name);
            ret = task();
            FinishTraceForSyncTask();
            return ret;
        }
        auto syncTask = [this, &ret, &task, &name] {
            StartTraceForSyncTask(name);
            ret = task();
            FinishTraceForSyncTask();
            ExecuteExportTask();
        };
        AppExecFwk::EventQueue::Priority priority = AppExecFwk::EventQueue::Priority::IMMEDIATE;
        static pid_t pid = getpid();
        if (pid == gettid()) {
            priority = AppExecFwk::EventQueue::Priority::VIP;
        }
        bool result = handler_->PostSyncTask(std::move(syncTask), "wms:" + name, priority);
        if (!result) {
            TLOGE(WmsLogTag::DEFAULT, "post task failed");
        }
        return ret;
    }

    void SetExportHandler(const std::shared_ptr<AppExecFwk::EventHandler>& handler);
    /*
     * Add export task, which will be executed after a task OS_SceneSession,
     * same name means same task, will be only executed once.
     */
    void AddExportTask(std::string taskName, Task&& task);

private:
    std::unordered_map<std::string, Task> exportFuncMap_; // ONLY Accessed in OS_SceneSession
    std::shared_ptr<AppExecFwk::EventHandler> exportHandler_;

protected:
    void ExecuteExportTask();
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_TASK_SCHEDULER_H
