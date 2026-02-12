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
constexpr uint32_t TEN_SECONDS = 10 * 1000;
constexpr uint32_t ONE_MINUTE  = 60 * 1000;
constexpr uint32_t ONE_HOUR    = 60 * 60 * 1000;

constexpr uint32_t RECORD_ONCE      = 1;
constexpr uint32_t RECORD_10_TIMES  = 10;
constexpr uint32_t RECORD_100_TIMES = 100;
constexpr uint32_t RECORD_200_TIMES = 200;

constexpr uint32_t ADDR_MASK = 0xFFFF;
constexpr uint32_t ADDR_MASK_WID = 0xFF;
constexpr uint32_t ADDR_SHIFT_24 = 24;
constexpr uint32_t ADDR_SHIFT_32 = 32;
constexpr uint32_t BITS_PER_BYTE = 8;
constexpr uint32_t LINE_SHIFT_16 = 16;
}
extern const std::unordered_set<WmsLogTag> TAG_WHITE_LIST;

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
    void* addr = __builtin_return_address(0);
    uintptr_t addr_int = reinterpret_cast<uintptr_t>(addr);
    
    return (sizeof(void*) == BITS_PER_BYTE) ?
        (addr_int << ADDR_SHIFT_32) | (__LINE__ & ADDR_MASK) :
        ((addr_int & ADDR_MASK) << LINE_SHIFT_16) | (__LINE__ & ADDR_MASK);
}

/**
 * Get the concatenation of the current function address, line number and window id
 * @param wid window id
 */
static inline uintptr_t GET_PACKED_ADDR_LINE_WID(uint32_t wid)
{
    void* addr = __builtin_return_address(0);
    uintptr_t addr_int = reinterpret_cast<uintptr_t>(addr);
    
    return (sizeof(void*) == BITS_PER_BYTE) ?
        (addr_int << ADDR_SHIFT_32) | ((__LINE__ & ADDR_MASK) << LINE_SHIFT_16) | (wid & ADDR_MASK) :
        ((addr_int & ADDR_MASK_WID) << ADDR_SHIFT_24) |
        ((__LINE__ & ADDR_MASK_WID) << LINE_SHIFT_16) |
        (wid & ADDR_MASK);
}

/**
 * @brief: limit printing log
 * @details TLOGI_LMT inner function
 */
#define TLOGI_LMT_INNER(functionAddress, timeWindowMs, maxCount, wid, tag, fmt, ...)              \
    do {                                                                                          \
	    if (TAG_WHITE_LIST.find(tag) != TAG_WHITE_LIST.end() &&                                   \
            RateLimitedLogger::getInstance()                                                      \
                .logFunction(functionAddress, timeWindowMs, maxCount)) {                          \
            TLOGI(tag, fmt, ##__VA_ARGS__);                                                       \
        }                                                                                         \
	} while (0)

/**
 * @brief: limit printing log
 * @details: usually use for client-side which not need to distinguish
 * @attention: only use for wms-layout
 */
#define TLOGI_LMT(timeWindowMs, maxCount, tag, fmt, ...)                                          \
    TLOGI_LMT_INNER(GET_PACKED_ADDR_LINE(), timeWindowMs, maxCount, tag, __VA_ARGS__)

/**
 * @brief: limit printing log
 * @details usually use for server-side to distinguish between different windows
 * @attention: only use for wms-layout
 */
#define TLOGI_LMTBYID(timeWindowMs, maxCount, wid, tag, fmt, ...)                                 \
    TLOGI_LMT_INNER(GET_PACKED_ADDR_LINE(wid), timeWindowMs, maxCount, tag, __VA_ARGS__)

/**
 * @brief: limit printing log
 * @details TLOGNI_LMT inner function
 */
#define TLOGNI_LMT_INNER(functionAddress, timeWindowMs, maxCount, wid, tag, fmt, ...)             \
    do {                                                                                          \
	    if (TAG_WHITE_LIST.find(tag) != TAG_WHITE_LIST.end() &&                                   \
		    RateLimitedLogger::getInstance()                                                      \
			    .logFunction(functionAddress, timeWindowMs, maxCount)) {                          \
		    TLOGNI(tag, fmt, ##__VA_ARGS__);                                                      \
	    }                                                                                         \
    } while (0)

/**
 * @brief limit printing log
 * @details usually use for client-side which not need to distinguish
 * @attention: only use for wms-layout
 */
#define TLOGNI_LMT(timeWindowMs, maxCount, tag, fmt, ...)                                         \
    TLOGNI_LMT_INNER(GET_PACKED_ADDR_LINE(), timeWindowMs, maxCount, tag, __VA_ARGS__)

/**
 * @brief limit printing log
 * @details use for server-side to distinguish between different windows
 * @attention: only use for wms-layout
 */
#define TLOGNI_LMTBYID(timeWindowMs, maxCount, wid, tag, fmt, ...)                                \
    TLOGNI_LMT_INNER(GET_PACKED_ADDR_LINE(wid), timeWindowMs, maxCount, tag, __VA_ARGS__)

} // namespace Rosen
} // namespace OHOS
#endif // RATE_LIMITED_LOGGER_H