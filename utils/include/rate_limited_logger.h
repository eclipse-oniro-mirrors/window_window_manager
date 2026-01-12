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

#ifndef RATE_LIMITED_LOGGER_H
#define RATE_LIMITED_LOGGER_H

#include <chrono>
#include <functional>
#include <mutex>
#include <parameters.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t TEN_SECONDS = 10 * 1000;
constexpr uint32_t ONE_MINUTE  = 60 * 1000;
constexpr uint32_t ONE_HOUR    = 60 * 60 * 1000;

constexpr uint32_t RECORD_ONCE      = 1;
constexpr uint32_t RECORD_10_TIMES  = 10;
constexpr uint32_t RECORD_100_TIMES = 100;
constexpr uint32_t RECORD_200_TIMES = 200;
}

class RateLimitedLogger {
private:
    struct FunctionRecord {
        int32_t count = 0;                                // Current count in time window
        std::chrono::steady_clock::time_point startTime;  // Time window start
    };

    std::unordered_map<std::string, FunctionRecord> functionRecords_;
    std::mutex functionRecordsMutex_;
    bool enabled_;

    // Private constructor for singleton
    RateLimitedLogger()
    {
        std::string parameterStr = OHOS::system::GetParameter("persist.window.windowLogLimit", "1");
        enabled_ = (parameterStr == "1");
    }
    
public:
    // Delete copy constructor and assignment operator
    RateLimitedLogger(const RateLimitedLogger&) = delete;
    RateLimitedLogger& operator=(const RateLimitedLogger&) = delete;

    // Get singleton instance
    static RateLimitedLogger& getInstance();

    /**
     * Log a message for a function with rate limiting
     * @param functionName Name of the function logging the message
     * @param timeWindowMs Time window in milliseconds
     * @param maxCount Maximum allowed logs in the time window
     * @param message The log message content
     * @return true if message was logged, false if rate limited
     */
    bool logFunction(const std::string& functionName, int32_t timeWindowMs, int32_t maxCount);

    /**
     * Enable or disable all logging
     * @param enabled true to enable logging, false to disable
     */
    void setEnabled(bool enabled);

    /**
     * Clear all rate limiting records
     */
    void clear();

    /**
     * Get current log count for a function in the current time window
     * @param functionName Name of the function
     * @return Current log count
     */
    int32_t getCurrentCount(const std::string& functionName);
};

#define TLOGI_LMT(timeWindowMs, maxCount, tag, fmt, ...)                                          \
    do {                                                                                          \
        if (RateLimitedLogger::getInstance().logFunction(__FUNCTION__, timeWindowMs, maxCount)) { \
            TLOGI(tag, fmt, ##__VA_ARGS__);                                                       \
        }                                                                                         \
    } while (0)

#define TLOGI_LMTKEY(timeWindowMs, maxCount, customKey, tag, fmt, ...)                            \
    do {                                                                                          \
        if (RateLimitedLogger::getInstance().logFunction(customKey, timeWindowMs, maxCount)) {    \
            TLOGI(tag, fmt, ##__VA_ARGS__);                                                       \
        }                                                                                         \
    } while (0)

#define TLOGI_LMTBYID(timeWindowMs, maxCount, wid, tag, fmt, ...)                                 \
    do {                                                                                          \
        if (RateLimitedLogger::getInstance().logFunction(__FUNCTION__ + std::to_string(wid),      \
            timeWindowMs, maxCount)) {                                                            \
            TLOGI(tag, fmt, ##__VA_ARGS__);                                                       \
        }                                                                                         \
    } while (0)

} // namespace Rosen
} // namespace OHOS
#endif // RATE_LIMITED_LOGGER_H