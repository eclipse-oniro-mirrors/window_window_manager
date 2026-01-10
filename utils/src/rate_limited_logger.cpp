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
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include "rate_limited_logger.h"
namespace OHOS {
namespace Rosen {
RateLimitedLogger& RateLimitedLogger::getInstance()
{
    static RateLimitedLogger instance_;
    return instance_;
}

bool RateLimitedLogger::logFunction(const std::string& functionName, int32_t timeWindowMs, int32_t maxCount)
{
    // Parameter abnormality
    if (timeWindowMs <= 0 || maxCount <= 0) {
        return false;
    }

    // Disable log rate limiting, always print logs
    if (!enabled_) {
        return true;
    }

    std::lock_guard<std::mutex> lock(functionRecordsMutex_);
    auto now = std::chrono::steady_clock::now();

    // Find or create function record
    auto& record = functionRecords_[functionName];
    
    // If new record or time window expired, reset count
    if (record.count == 0 ||
        std::chrono::duration_cast<std::chrono::milliseconds>(now - record.startTime).count() >= timeWindowMs) {
        record.count = 0;
        record.startTime = now;
    }

    // Check if within limit
    if (record.count < maxCount) {
        record.count++;
        return true;
    }
    // Exceeded limit, don't log
    return false;
}

void RateLimitedLogger::clear()
{
    std::lock_guard<std::mutex> lock(functionRecordsMutex_);
    functionRecords_.clear();
}

void RateLimitedLogger::setEnabled(bool enabled)
{
    enabled_ = enabled;
}

int32_t RateLimitedLogger::getCurrentCount(const std::string& functionName)
{
    std::lock_guard<std::mutex> lock(functionRecordsMutex_);
    auto it = functionRecords_.find(functionName);
    return (it != functionRecords_.end()) ? it->second.count : 0;
}
} // namespace Rosen
} // namespace OHOS