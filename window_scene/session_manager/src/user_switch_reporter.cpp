/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "user_switch_reporter.h"

#include <chrono>
#include <cinttypes>
#include <hisysevent.h>

#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
UserSwitchReporter::UserSwitchReporter(UserSwitchEventType type, bool isUserActive)
    : startTime_(GetCurrentTimeMillis()), switchEventType_(type), isUserActive_(isUserActive)
{
}

UserSwitchReporter::~UserSwitchReporter()
{
    if (!ReportSwitchDuration()) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "User switch duration reporting failed.");
    }
}

uint64_t UserSwitchReporter::GetCurrentTimeMillis()
{
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
}

uint8_t UserSwitchReporter::GetSwitchState() const
{
    /**
     * Note: Based on switchEventType_ and isUserActive_ to get user switch state.
     * switchEventType_ -> 0: switching / 1: switched
     * isUserActive_ -> false: switch out / true: switch in
     *
     * { switchEventType_ = 0, isUserActive_ = false },   // 00 -> 0
     * { switchEventType_ = 0, isUserActive_ = true },    // 01 -> 1
     * { switchEventType_ = 1, isUserActive_ = false },   // 10 -> 2
     * { switchEventType_ = 1, isUserActive_ = true },    // 11 -> 3
     */
    return (static_cast<uint8_t>(switchEventType_) << 1) | static_cast<uint8_t>(isUserActive_);
}

bool UserSwitchReporter::ReportSwitchDuration() const
{
    // Convert timestamp to YYYY-MM-DD-HH-MM-SS format.
    auto convertToReadableTime = [](uint64_t utc_timestamp) -> uint64_t {
        // Second-level timestamp converted to Beijing Time (UTC+8)
        time_t to_seconds = static_cast<time_t>(utc_timestamp / 1000ULL);
        std::tm beijing_tm = {};
        localtime_r(&to_seconds, &beijing_tm);

        // Format as a readable string (YYYY-MM-DD HH:MM:SS)
        char buffer[50];
        std::strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &beijing_tm);
        auto readableTime = std::string(buffer);
        TLOGD(WmsLogTag::WMS_MULTI_USER, "After convert time=%{public}s", readableTime.c_str());
        return std::stoul(readableTime);
    };

    auto endTime = GetCurrentTimeMillis();
    auto duration = static_cast<uint32_t>(endTime - startTime_);
    auto switchState = GetSwitchState();
    TLOGD(WmsLogTag::WMS_MULTI_USER,
          "startTime: %{public}" PRIu64 ", endTime: %{public}" PRIu64 ", duration: %{public}u ms",
          startTime_, endTime, duration);

    int32_t ret = HiSysEventWrite(
        HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,  // domain
        "USER_SWITCH_DURATION",                         // name
        HiviewDFX::HiSysEvent::EventType::STATISTIC,    // eventType
        "SWITCH_TYPE", switchState,
        "START_TIME", convertToReadableTime(startTime_),
        "END_TIME", convertToReadableTime(endTime),
        "DURATION", duration);
    if (ret != 0) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Failed to write HiSysEvent, ret: %{public}d", ret);
        return false;
    }
    return true;
}
} // namespace Rosen
} // namespace OHOS
