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

#include "ffrt_queue_helper.h"

#include <cstdint>
#include <hisysevent.h>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "ffrt_inner.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr uint32_t FFRT_STAT_MAX_COUNT = 5000;
constexpr uint64_t FFRT_STAT_INTERVAL_MS = 5 * 60 * 1000;
constexpr const char* DIGITS = "0123456789";

struct TaskStatAggregation {
    uint64_t totalTime = 0;
    uint64_t callCount = 0;
};

inline uint64_t ConvertCntvctToUs(uint64_t ticks)
{
#if defined(__aarch64__)
    uint64_t freq = 1;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
    return (ticks * 1000000) / freq;
#else
    return ticks;
#endif
}

void ReportXpower(const std::unordered_map<std::string, TaskStatAggregation>& aggregated,
    uint64_t minStartTime, uint64_t maxEndTime)
{
    std::vector<std::string> taskNames;
    std::vector<int64_t> durations;
    std::vector<int64_t> counts;
    std::vector<int32_t> pids;
    int32_t pid = static_cast<int32_t>(getpid());
    taskNames.reserve(aggregated.size());
    durations.reserve(aggregated.size());
    counts.reserve(aggregated.size());
    pids.reserve(aggregated.size());
    for (const auto& [name, stat] : aggregated) {
        taskNames.push_back(name);
        durations.push_back(static_cast<int64_t>(ConvertCntvctToUs(stat.totalTime) / 1000));
        counts.push_back(static_cast<int64_t>(stat.callCount));
        pids.push_back(pid);
    }
    int32_t ret = HiSysEventWrite("POWERTHERMAL", "FFRT_TASK_INFO",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "START_TIME", static_cast<int64_t>(ConvertCntvctToUs(minStartTime)),
        "END_TIME", static_cast<int64_t>(ConvertCntvctToUs(maxEndTime)),
        "TASKNAME", taskNames,
        "DURATION", durations,
        "COUNT", counts,
        "PID", pids);
    if (ret != 0) {
        TLOGNE(WmsLogTag::WMS_LIFE, "ffrt statistics: ReportXpower: HiSysEventWrite failed, ret=%{public}d", ret);
    }
}
} // namespace

std::string StripGid(std::string label)
{
    if (label.rfind("sq_", 0) != 0) {
        return label;
    }
    size_t p = label.rfind('_');
    if (p != std::string::npos && p + 1 < label.size() &&
        label.find_first_not_of(DIGITS, p + 1) == std::string::npos) {
        label.erase(p);
    }
    return label;
}

constexpr int32_t FFRT_USER_INTERACTIVE_MAX_THREAD_NUM = 5;

FfrtQueueHelper::FfrtQueueHelper()
{
    ffrtQueue_ = std::make_unique<ffrt::queue>(ffrt::queue_concurrent, "FfrtQueueHelper",
        ffrt::queue_attr()
            .qos(ffrt_qos_user_interactive)
            .max_concurrency(FFRT_USER_INTERACTIVE_MAX_THREAD_NUM));
}

FfrtQueueHelper::~FfrtQueueHelper()
{
    StopStatCollection();
}

// LCOV_EXCL_START
bool FfrtQueueHelper::SubmitTaskAndWait(std::function<void()>&& task, uint64_t timeout)
{
    auto timeoutFuture = std::make_shared<TimeoutFuture<bool>>();
    ffrtQueue_->submit([localTask = std::move(task), timeoutFuture, where = __func__] {
        if (localTask) {
            localTask();
        } else {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: task function is empty", where);
        }
        timeoutFuture->FutureCall(true);
    }, __func__);
    bool isTimeout = false;
    timeoutFuture->GetResult(timeout, isTimeout);
    return isTimeout;
}

bool FfrtQueueHelper::SubmitTaskAndWait(std::function<void()>&& task, uint64_t timeout, const std::string& name)
{
    auto timeoutFuture = std::make_shared<TimeoutFuture<bool>>();
    ffrtQueue_->submit([localTask = std::move(task), timeoutFuture, where = __func__] {
        if (localTask) {
            localTask();
        } else {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: task function is empty", where);
        }
        timeoutFuture->FutureCall(true);
    }, name.c_str());
    bool isTimeout = false;
    timeoutFuture->GetResult(timeout, isTimeout);
    return isTimeout;
}

void FfrtQueueHelper::SubmitTask(std::function<void()>&& task)
{
    ffrtQueue_->submit(task);
}

void FfrtQueueHelper::SubmitTaskToHead(std::function<void()>&& task)
{
    ffrtQueue_->submit_head(task);
}

void FfrtQueueHelper::SubmitTask(std::function<void()>&& task, const std::string& name)
{
    ffrtQueue_->submit(task, name.c_str());
}

void FfrtQueueHelper::SubmitTaskToHead(std::function<void()>&& task, const std::string& name)
{
    ffrtQueue_->submit_head(task, name.c_str());
}
// LCOV_EXCL_STOP

void FfrtQueueHelper::StartStatCollection()
{
    TLOGNI(WmsLogTag::WMS_LIFE, "ffrt statistics: StartStatCollection enter");
    if (statEnabled_.exchange(true)) {
        TLOGNW(WmsLogTag::WMS_LIFE, "ffrt statistics: StartStatCollection: already started");
        return;
    }
    statBufferSize_ = FFRT_STAT_MAX_COUNT * static_cast<uint32_t>(sizeof(ffrt_stat));
    statBuffer_ = std::make_unique<char[]>(statBufferSize_);
    if (statBuffer_ == nullptr) {
        TLOGNE(WmsLogTag::WMS_LIFE, "ffrt statistics: StartStatCollection: alloc buffer failed, size=%{public}u", statBufferSize_);
        statEnabled_ = false;
        statBufferSize_ = 0;
        return;
    }
    if (ffrt_dump(DUMP_START_STAT, statBuffer_.get(), statBufferSize_) != 0) {
        TLOGNE(WmsLogTag::WMS_LIFE, "ffrt statistics: StartStatCollection: ffrt_dump start failed");
        statBuffer_.reset();
        statEnabled_ = false;
        statBufferSize_ = 0;
        return;
    }
    StartStatTimer();
}

void FfrtQueueHelper::StartStatTimer()
{
    statTimer_ = ffrt_timer_start(ffrt_qos_default, FFRT_STAT_INTERVAL_MS, this,
        &FfrtQueueHelper::OnStatTimer, true);
    if (statTimer_ == ffrt_error) {
        TLOGNE(WmsLogTag::WMS_LIFE, "ffrt statistics: StartStatTimer: ffrt_timer_start failed");
    }
}

void FfrtQueueHelper::OnStatTimer(void* data)
{
    auto* self = static_cast<FfrtQueueHelper*>(data);
    if (self != nullptr) {
        self->ParseAndReportStat();
    }
}

void FfrtQueueHelper::ParseAndReportStat()
{
    TLOGNI(WmsLogTag::WMS_LIFE, "ffrt statistics: ParseAndReportStat enter");
    if (!statEnabled_ || statBuffer_ == nullptr || statBufferSize_ == 0) {
        return;
    }
    if (ffrt_dump(DUMP_STOP_STAT, statBuffer_.get(), statBufferSize_) != 0) {
        TLOGNE(WmsLogTag::WMS_LIFE, "ffrt statistics: ParseAndReportStat: ffrt_dump stop failed");
    }
    std::unordered_map<std::string, TaskStatAggregation> aggregated;
    uint64_t minStartTime = UINT64_MAX;
    uint64_t maxEndTime = 0;
    const auto* stats = reinterpret_cast<const ffrt_stat*>(statBuffer_.get());
    const uint32_t statCount = statBufferSize_ / static_cast<uint32_t>(sizeof(ffrt_stat));
    uint32_t validCount = 0;
    for (uint32_t i = 0; i < statCount; ++i) {
        const auto& s = stats[i];
        if (s.startTime == 0 || s.endTime == 0 || s.endTime < s.startTime) {
            continue;
        }
        ++validCount;
        std::string raw(s.taskName);
        while (!raw.empty() && raw.back() == ' ') {
            raw.pop_back();
        }
        if (raw.find("OS_TaskWorker") != std::string::npos) {
            continue;
        }
        std::string name = StripGid(std::move(raw));
        auto& entry = aggregated[name];
        entry.totalTime += (s.endTime - s.startTime);
        entry.callCount += 1;
        if (s.startTime < minStartTime) {
            minStartTime = s.startTime;
        }
        if (s.endTime > maxEndTime) {
            maxEndTime = s.endTime;
        }
    }
    TLOGNI(WmsLogTag::WMS_LIFE, "ffrt statistics: %{public}u/%{public}u records parsed",
        validCount, statCount);
    if (minStartTime != UINT64_MAX) {
        ReportXpower(aggregated, minStartTime, maxEndTime);
        TLOGNI(WmsLogTag::WMS_LIFE,
            "ffrt statistics window: %{public}u unique tasks, start=%{public}llu end=%{public}llu",
            static_cast<uint32_t>(aggregated.size()),
            static_cast<unsigned long long>(minStartTime),
            static_cast<unsigned long long>(maxEndTime));
        for (const auto& [name, stat] : aggregated) {
            TLOGND(WmsLogTag::WMS_LIFE, "ffrt statistics: task=%{public}s count=%{public}llu total=%{public}llu",
                name.c_str(), static_cast<unsigned long long>(stat.callCount),
                static_cast<unsigned long long>(stat.totalTime));
        }
    }
    if (ffrt_dump(DUMP_START_STAT, statBuffer_.get(), statBufferSize_) != 0) {
        TLOGNE(WmsLogTag::WMS_LIFE, "ffrt statistics: ParseAndReportStat: ffrt_dump restart failed");
        statEnabled_ = false;
    }
}

void FfrtQueueHelper::StopStatCollection()
{
    TLOGNI(WmsLogTag::WMS_LIFE, "ffrt statistics: StopStatCollection enter");
    if (statTimer_ != ffrt_error) {
        ffrt_timer_stop(ffrt_qos_default, statTimer_);
        statTimer_ = ffrt_error;
    }
    if (statEnabled_ && statBuffer_ != nullptr) {
        ffrt_dump(DUMP_STOP_STAT, statBuffer_.get(), statBufferSize_);
    }
    statEnabled_ = false;
    statBuffer_.reset();
    statBufferSize_ = 0;
}
} // namespace OHOS::Rosen
