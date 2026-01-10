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
#include <unordered_set>
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
inline const std::unordered_set<std::string> tagWhiteList = {"WMS_LAYOUT"};
constexpr uint32_t TEN_SECONDS = 10 * 1000;
constexpr uint32_t ONE_MINUTE  = 60 * 1000;
constexpr uint32_t ONE_HOUR    = 60 * 60 * 1000;

constexpr uint32_t RECORD_ONCE      = 1;
constexpr uint32_t RECORD_10_TIMES  = 10;
constexpr uint32_t RECORD_100_TIMES = 100;
constexpr uint32_t RECORD_200_TIMES = 200;

constexpr uint32_t BITS_PER_BYTE = 8;
constexpr uint32_t ADDR_SHIFT_64 = 32;
constexpr uint32_t ADDR_MASK_32 = 0xFFFF;
constexpr uint32_t LINE_MASK_32 = 0xFFFF;
constexpr uint32_t ADDR_MASK_32_WID = 0xFF;
constexpr uint32_t LINE_MASK_32_WID = 0xFF;
constexpr uint32_t WID_MASK = 0xFFFF;
constexpr uint32_t LINE_SHIFT_16 = 16;
constexpr uint32_t ADDR_SHIFT_32_WID = 24;
constexpr uint32_t LINE_SHIFT_32_WID = 16;
}

class RateLimitedLogger {
private:
    struct FunctionRecord {
        int32_t count = 0;                                // Current count in time window
        std::chrono::steady_clock::time_point startTime;  // Time window start
    };

    std::unordered_map<std::uintptr_t, FunctionRecord> functionRecords_;
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
     * @param functionAddress address and line of the function logging the message
     * @param timeWindowMs Time window in milliseconds
     * @param maxCount Maximum allowed logs in the time window
     * @param message The log message content
     * @return true if message was logged, false if rate limited
     */
    bool logFunction(const std::uintptr_t& functionAddress, uint32_t timeWindowMs, uint32_t maxCount);

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
     * @param functionAddress address and line of the function
     * @return Current log count
     */
    int32_t getCurrentCount(const std::uintptr_t& functionAddress);
};

/**
 * Get the concatenation of the current function address and line number
 */
static inline uintptr_t GET_PACKED_ADDR_LINE()
{
    void* addr;
    __asm__("1:\n mov $1b, %0" : "=r"(addr));
    
    return (sizeof(void*) == BITS_PER_BYTE)
        ? (reinterpret_cast<uintptr_t>(addr) << ADDR_SHIFT_64) | __LINE__
        : ((reinterpret_cast<uintptr_t>(addr) & ADDR_MASK_32) << LINE_SHIFT_16) | (__LINE__ & LINE_MASK_32);
}

/**
 * Get the concatenation of the current function address, line number and window id
 * @param wid window id
 */
static inline uintptr_t GET_PACKED_ADDR_LINE_WID(uint32_t wid)
{
    void* addr = __builtin_return_address(0);
    uintptr_t addr_int = reinterpret_cast<uintptr_t>(addr);
    
    return (sizeof(void*) == BITS_PER_BYTE)
        ? (addr_int << ADDR_SHIFT_64) | ((__LINE__ & LINE_MASK_32) << LINE_SHIFT_16) | (wid & WID_MASK)
        : ((addr_int & ADDR_MASK_32_WID) << ADDR_SHIFT_32_WID) |
        ((__LINE__ & LINE_MASK_32_WID) << LINE_SHIFT_32_WID) | (wid & WID_MASK);
}

/**
* usually use for client-side which not need to distinguish
*/
#define TLOGI_LMT(timeWindowMs, maxCount, tag, fmt, ...)                                          \
    do {                                                                                          \
        uintptr_t functionAddress = GET_PACKED_ADDR_LINE();                                       \
        if (tagWhiteList.find(std::string(tag)) != tagWhiteList.end() &&                          \
            RateLimitedLogger::getInstance().logFunction(functionAddress,                         \
            timeWindowMs, maxCount)) {                                                            \
            TLOGI(tag, fmt, ##__VA_ARGS__);                                                       \
        }                                                                                         \
    } while (0)

/**
* usually use for server-side to distinguish between different screens
*/
#define TLOGI_LMTBYID(timeWindowMs, maxCount, wid, tag, fmt, ...)                                 \
    do {                                                                                          \
        uintptr_t functionAddress = GET_PACKED_ADDR_LINE_WID(wid);                                \
        if (tagWhiteList.find(std::string(tag)) != tagWhiteList.end() &&                          \
            RateLimitedLogger::getInstance().logFunction(functionAddress,                         \
            timeWindowMs, maxCount)) {                                                            \
            TLOGI(tag, fmt, ##__VA_ARGS__);                                                       \
        }                                                                                         \
    } while (0)

} // namespace Rosen
} // namespace OHOS
#endif // RATE_LIMITED_LOGGER_H