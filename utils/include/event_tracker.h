/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_SCREEN_EVENT_TRACKER_H
#define OHOS_ROSEN_SCREEN_EVENT_TRACKER_H

#include <iomanip>
#include <list>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
const int32_t OUTPUT_FREQ = 1; // 1Hz
const int32_t MAX_CAPACITY = 6;

struct TrackInfo {
    std::string info;
    std::chrono::system_clock::time_point timestamp;
};

class EventTracker {
public:
    void RecordEvent(std::string info = "")
    {
        std::lock_guard<std::mutex> lock(mutex_);
        recordInfos_.push_back({info, std::chrono::system_clock::now()});
    }

    void RecordBoundsEvent(std::string info = "")
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (recordBoundsInfos_.size() >= MAX_CAPACITY) {
            recordBoundsInfos_.pop_front();
        }
        recordBoundsInfos_.push_back({info, std::chrono::system_clock::now()});
    }

    void ClearAllRecordedEvents()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        recordInfos_.clear();
        recordBoundsInfos_.clear();
    }

    void LogWarningAllInfos() const
    {
        auto now = std::chrono::system_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastOutputTime_).count() < OUTPUT_FREQ) {
            return ; // Output too frequent. Try again later.
        }
        lastOutputTime_ = now;
        
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& info : recordInfos_) {
            TLOGE_LIMITN_MIN(WmsLogTag::DMS, THREE_TIMES, "[%{public}s]: %{public}s",
                formatTimestamp(info.timestamp).c_str(), info.info.c_str());
        }

        for (const auto& info : recordBoundsInfos_) {
            TLOGE_LIMITN_MIN(WmsLogTag::DMS, THREE_TIMES, "[%{public}s]: %{public}s",
                formatTimestamp(info.timestamp).c_str(), info.info.c_str());
        }
    }

    std::string formatTimestamp(const std::chrono::system_clock::time_point& timePoint) const
    {
        const int32_t WIDTH_TIME = 2;
        const int32_t WIDTH_TIME_MS = 3;
        const int32_t TIME_CONVERT_MS = 1000;
        const char DEFAULT_CHAR = '0';
        auto time = std::chrono::system_clock::to_time_t(timePoint);
        std::tm localTime;
        localtime_r(&time, &localTime);
        auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            timePoint.time_since_epoch()) % TIME_CONVERT_MS;

        std::ostringstream oss;
        oss << std::setfill(DEFAULT_CHAR)
            << std::setw(WIDTH_TIME) << (localTime.tm_mon + 1) << '-'
            << std::setw(WIDTH_TIME) << localTime.tm_mday << ' '
            << std::setw(WIDTH_TIME) << localTime.tm_hour << ':'
            << std::setw(WIDTH_TIME) << localTime.tm_min << ':'
            << std::setw(WIDTH_TIME) << localTime.tm_sec << '.'
            << std::setw(WIDTH_TIME_MS) << timeMs.count();
        return oss.str();
    }

    const std::vector<TrackInfo>& GetRecordInfos()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        allRecordInfos_ = recordInfos_;
        for (auto info : recordBoundsInfos_) {
            allRecordInfos_.emplace_back(info);
        }
        return allRecordInfos_;
    }

private:
    mutable std::mutex mutex_;
    mutable std::chrono::system_clock::time_point lastOutputTime_;
    std::vector<TrackInfo> recordInfos_;
    std::list<TrackInfo> recordBoundsInfos_;
    std::vector<TrackInfo> allRecordInfos_;
};


} // Rosen
} // OHOS
#endif // OHOS_ROSEN_SCREEN_EVENT_TRACKER_H