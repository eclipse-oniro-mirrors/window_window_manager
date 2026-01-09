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

#ifndef WINDOW_WINDOW_MANAGER_UTILS_DMS_GLOBAL_MUTEX_H
#define WINDOW_WINDOW_MANAGER_UTILS_DMS_GLOBAL_MUTEX_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <thread>
#include "window_manager_hilog.h"

namespace OHOS::Rosen {

enum class IPCPriority : uint32_t {
    // The highest priority, try_lock wait 5ms
    LOW = 0,
    // The highest priority, skip try_lock
    VIP,
};

namespace DmUtils {
#define FREE_GLOBAL_LOCK_FOR_IPC() {DmUtils::DropLock ipcLock;}
class HoldLock {
public:
    HoldLock(IPCPriority IPCPriority = IPCPriority::LOW)
    {
        // "counter" is used to resolve deadlock issues caused by nested tasks within the same thread.
        // "lockStatus" is used to prevent task delay canuse by prolonged lock retention.
        if (++counter() == 1) {
            // max try lock for 2 times, 1 second
            // if lock success, execute Serial Run Mode, else roll back to Concurrent Run Mode
            if ((resMtx.try_lock()) ||
                (IPCPriority < IPCPriority::VIP && lockStatus && resMtx.try_lock_for(TRY_LOCK_TIMEOUT))) {
                lockStatus = true;
            } else {
                lockStatus = false;
                TLOGD(WmsLogTag::DMS, "[HoldLock] try lock fail");
            }
        }
    }

    ~HoldLock()
    {
        if (--counter() == 0 && lockStatus) {
            resMtx.unlock();
        }
    }
    static bool ForceUnlock()
    {
        auto threadLocked = --counter() >= 0;
        if (threadLocked && lockStatus) {
            resMtx.unlock();
            return true;
        }
        return false;
    }

    static void ForceLock(bool hasLock)
    {
        ++counter();
        if (hasLock) {
            if ((resMtx.try_lock()) || (lockStatus && resMtx.try_lock_for(TRY_LOCK_TIMEOUT))) {
                lockStatus = true;
            } else {
                lockStatus = false;
                TLOGD(WmsLogTag::DMS, "[ForceLock] try lock fail");
            }
        }
    }

private:
    static std::timed_mutex resMtx;
    static thread_local bool lockStatus;
    static constexpr std::chrono::milliseconds TRY_LOCK_TIMEOUT{ 50 };
    static int& counter()
    {
        static thread_local int _count = 0;
        return _count;
    }
};

class DropLock {
public:
    DropLock()
    {
        hasLock = HoldLock::ForceUnlock();
    }

    ~DropLock()
    {
        HoldLock::ForceLock(hasLock);
    }

private:
    bool hasLock = true;
};

template <typename Func> auto wrap_callback(Func func)
{
    return [func](auto&&... args) {
        HoldLock callback_lock;
        func(std::forward<decltype(args)>(args)...);
    };
}

template <typename Rep, typename Period>
std::cv_status safe_wait_for(std::condition_variable& cv,
                             std::unique_lock<std::mutex>& lock,
                             const std::chrono::duration<Rep, Period>& rel_time)
{
    DropLock unlockGuard;
    auto status = cv.wait_for(lock, rel_time);
    return status;
}

template <typename Rep, typename Period, typename Predicate>
bool safe_wait_for(std::condition_variable& cv,
                   std::unique_lock<std::mutex>& lock,
                   const std::chrono::duration<Rep, Period>& rel_time,
                   Predicate pred)
{
    DropLock unlockGuard;
    bool result = cv.wait_for(lock, rel_time, pred);
    return result;
}

} // namespace DmUtils
} // namespace OHOS::Rosen
#endif // WINDOW_WINDOW_MANAGER_UTILS_DMS_GLOBAL_MUTEX_H