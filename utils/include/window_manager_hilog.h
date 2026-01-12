/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_WM_INCLUDE_WINDOW_MANAGER_HILOG_H
#define OHOS_WM_INCLUDE_WINDOW_MANAGER_HILOG_H

#include <cstdint>
#include <unordered_map>
#include "hilog/log.h"

namespace OHOS {
namespace Rosen {
static constexpr unsigned int HILOG_DOMAIN_WINDOW = 0xD004200;
static constexpr unsigned int HILOG_DOMAIN_DISPLAY = 0xD004201;

#define PRINT_WLOG(level, ...) HILOG_IMPL(LABEL.type, level, LABEL.domain, LABEL.tag, ##__VA_ARGS__)

#define WLOGD(fmt, ...) PRINT_WLOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define WLOGI(fmt, ...) PRINT_WLOG(LOG_INFO, fmt, ##__VA_ARGS__)
#define WLOGW(fmt, ...) PRINT_WLOG(LOG_WARN, fmt, ##__VA_ARGS__)
#define WLOGE(fmt, ...) PRINT_WLOG(LOG_ERROR, fmt, ##__VA_ARGS__)

#define C_W_FUNC __func__

#define WLOGFD(fmt, ...) WLOGD("%{public}s: " fmt, C_W_FUNC, ##__VA_ARGS__)
#define WLOGFI(fmt, ...) WLOGI("%{public}s: " fmt, C_W_FUNC, ##__VA_ARGS__)
#define WLOGFW(fmt, ...) WLOGW("%{public}s: " fmt, C_W_FUNC, ##__VA_ARGS__)
#define WLOGFE(fmt, ...) WLOGE("%{public}s: " fmt, C_W_FUNC, ##__VA_ARGS__)

#define WIN_LOG_LIMIT_HOURS 3600
#define WIN_LOG_LIMIT_MINUTE 60
#define THREE_TIMES 3
#define TEN_TIMES 10

enum class WmsLogTag : uint8_t {
    DEFAULT = 0,               // C04200
    DMS,                       // C04201
    WMS_MAIN,                  // C04202
    WMS_SUB,                   // C04203
    WMS_SCB,                   // C04204
    WMS_DIALOG,                // C04205
    WMS_SYSTEM,                // C04206
    WMS_FOCUS,                 // C04207
    WMS_LAYOUT,                // C04208
    WMS_IMMS,                  // C04209
    WMS_LIFE,                  // C0420A
    WMS_KEYBOARD,              // C0420B
    WMS_EVENT,                 // C0420C
    WMS_UIEXT,                 // C0420D
    WMS_PIP,                   // C0420E
    WMS_RECOVER,               // C0420F
    WMS_MULTI_USER,            // C04210
    WMS_TOAST,                 // C04211
    WMS_MULTI_WINDOW,          // C04212
    WMS_INPUT_KEY_FLOW,        // C04213
    WMS_PIPELINE,              // C04214
    WMS_HIERARCHY,             // C04215
    WMS_PATTERN,               // C04216
    WMS_ATTRIBUTE,             // C04217
    WMS_PC,                    // C04218
    WMS_DECOR,                 // C04219
    WMS_LAYOUT_PC,             // C0421A
    WMS_STARTUP_PAGE,          // C0421B
    WMS_COMPAT,                // C0421C
    WMS_ROTATION,              // C0421D
    WMS_ANIMATION,             // C0421E
    END,                       // Last one, do not use
};

extern const char* g_domainContents[static_cast<uint32_t>(WmsLogTag::END)];
#ifdef IS_RELEASE_VERSION
#define WMS_FILE_NAME ""
#define FMT_PREFIX "%{public}s%{public}s: "
#define FMT_PREFIX_NO_FUNC "%{public}s"
#else
#define WMS_FILE_NAME (__builtin_strrchr(__FILE_NAME__, '/') ?   \
                       __builtin_strrchr(__FILE_NAME__, '/') + 1 : __FILE_NAME__)
#define FMT_PREFIX "%{public}s %{public}s: "
#define FMT_PREFIX_NO_FUNC "%{public}s: "
#endif

#define PRINT_TLOG(level, tag, ...)                                                                     \
    do {                                                                                                \
        uint32_t hilogDomain = HILOG_DOMAIN_WINDOW + static_cast<uint32_t>(tag);                        \
        const char *domainContent = (tag >= WmsLogTag::DEFAULT && tag < WmsLogTag::END) ?               \
            g_domainContents[static_cast<uint32_t>(tag)] : "";                                          \
        HILOG_IMPL(LOG_CORE, level, hilogDomain, domainContent, ##__VA_ARGS__);                         \
    } while (0)

#define TLOGD(tag, fmt, ...) \
PRINT_TLOG(LOG_DEBUG, tag, FMT_PREFIX fmt, WMS_FILE_NAME, C_W_FUNC, ##__VA_ARGS__)
#define TLOGI(tag, fmt, ...) \
PRINT_TLOG(LOG_INFO, tag, FMT_PREFIX fmt, WMS_FILE_NAME, C_W_FUNC, ##__VA_ARGS__)
#define TLOGW(tag, fmt, ...) \
PRINT_TLOG(LOG_WARN, tag, FMT_PREFIX fmt, WMS_FILE_NAME, C_W_FUNC, ##__VA_ARGS__)
#define TLOGE(tag, fmt, ...) \
PRINT_TLOG(LOG_ERROR, tag, FMT_PREFIX fmt, WMS_FILE_NAME, C_W_FUNC, ##__VA_ARGS__)

/*
 * There is no function name built in TLOGN log micros. Choose suitable log micros when needed.
 */
#define TLOGND(tag, fmt, ...) \
PRINT_TLOG(LOG_DEBUG, tag, FMT_PREFIX_NO_FUNC fmt, WMS_FILE_NAME, ##__VA_ARGS__)
#define TLOGNI(tag, fmt, ...) \
PRINT_TLOG(LOG_INFO, tag, FMT_PREFIX_NO_FUNC fmt, WMS_FILE_NAME, ##__VA_ARGS__)
#define TLOGNW(tag, fmt, ...) \
PRINT_TLOG(LOG_WARN, tag, FMT_PREFIX_NO_FUNC fmt, WMS_FILE_NAME, ##__VA_ARGS__)
#define TLOGNE(tag, fmt, ...) \
PRINT_TLOG(LOG_ERROR, tag, FMT_PREFIX_NO_FUNC fmt, WMS_FILE_NAME, ##__VA_ARGS__)


#define WIN_PRINT_LIMIT(tag, level, intervals, canPrint, frequency)                                      \
    do {                                                                                                 \
        uint32_t hilogDomain = HILOG_DOMAIN_WINDOW + static_cast<uint32_t>(tag);                         \
        const char *domainContent = ((tag) >= WmsLogTag::DEFAULT && (tag) < WmsLogTag::END) ?            \
            g_domainContents[static_cast<uint32_t>(tag)] : "";                                           \
        static auto last = std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>();   \
        static uint32_t supressed = 0;                                                                   \
        static int printCount = 0;                                                                       \
        auto now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()); \
        auto duration = now - last;                                                                      \
        if (duration.count() >= (intervals)) {                                                           \
            last = now;                                                                                  \
            uint32_t supressedCnt = supressed;                                                           \
            supressed = 0;                                                                               \
            printCount = 1;                                                                              \
            if (supressedCnt != 0) {                                                                     \
                ((void)HILOG_IMPL(LOG_CORE, (level), hilogDomain, domainContent,                         \
                    "%{public}s log suppressed cnt %{public}u", __func__, supressedCnt));                \
            }                                                                                            \
            (canPrint) = true;                                                                           \
        } else {                                                                                         \
            if ((printCount++) < (frequency)) {                                                          \
                (canPrint) = true;                                                                       \
            } else {                                                                                     \
                supressed++;                                                                             \
                (canPrint) = false;                                                                      \
            }                                                                                            \
        }                                                                                                \
    } while (0)

#define TLOGI_LIMITN_HOUR(tag, freq, fmt, ...)                                                           \
    do {                                                                                                 \
        bool can = true;                                                                                 \
        TLOGD(tag, fmt, ##__VA_ARGS__);                                                                  \
        WIN_PRINT_LIMIT(tag, LOG_INFO, WIN_LOG_LIMIT_HOURS, can, freq);                                  \
        if (can) {                                                                                       \
            TLOGI(tag, fmt, ##__VA_ARGS__);                                                              \
        }                                                                                                \
    } while (0)

#define TLOGW_LIMITN_HOUR(tag, freq, fmt, ...)                                                           \
    do {                                                                                                 \
        bool can = true;                                                                                 \
        TLOGD(tag, fmt, ##__VA_ARGS__);                                                                  \
        WIN_PRINT_LIMIT(tag, LOG_WARN, WIN_LOG_LIMIT_HOURS, can, freq);                                  \
        if (can) {                                                                                       \
            TLOGW(tag, fmt, ##__VA_ARGS__);                                                              \
        }                                                                                                \
    } while (0)

#define TLOGE_LIMITN_HOUR(tag, freq, fmt, ...)                                                           \
    do {                                                                                                 \
        bool can = true;                                                                                 \
        TLOGD(tag, fmt, ##__VA_ARGS__);                                                                  \
        WIN_PRINT_LIMIT(tag, LOG_ERROR, WIN_LOG_LIMIT_HOURS, can, freq);                                 \
        if (can) {                                                                                       \
            TLOGE(tag, fmt, ##__VA_ARGS__);                                                              \
        }                                                                                                \
    } while (0)

#define TLOGI_LIMITN_MIN(tag, freq, fmt, ...)                                                            \
    do {                                                                                                 \
        bool can = true;                                                                                 \
        OGD(tag, fmt, ##__VA_ARGS__);                                                                    \
        WIN_PRINT_LIMIT(tag, LOG_INFO, WIN_LOG_LIMIT_MINUTE, can, freq);                                 \
        if (can) {                                                                                       \
            TLOGI(tag, fmt, ##__VA_ARGS__);                                                              \
        }                                                                                                \
    } while (0)

#define TLOGW_LIMITN_MIN(tag, freq, fmt, ...)                                                            \
    do {                                                                                                 \
        bool can = true;                                                                                 \
        TLOGD(tag, fmt, ##__VA_ARGS__);                                                                  \
        WIN_PRINT_LIMIT(tag, LOG_WARN, WIN_LOG_LIMIT_MINUTE, can, freq);                                 \
        if (can) {                                                                                       \
            TLOGW(tag, fmt, ##__VA_ARGS__);                                                              \
        }                                                                                                \
    } while (0)

#define TLOGE_LIMITN_MIN(tag, freq, fmt, ...)                                                            \
    do {                                                                                                 \
        bool can = true;                                                                                 \
        TLOGD(tag, fmt, ##__VA_ARGS__);                                                                  \
        WIN_PRINT_LIMIT(tag, LOG_ERROR, WIN_LOG_LIMIT_MINUTE, can, freq);                                \
        if (can) {                                                                                       \
            TLOGE(tag, fmt, ##__VA_ARGS__);                                                              \
        }                                                                                                \
    } while (0)

} // namespace OHOS
}
#endif // FRAMEWORKS_WM_INCLUDE_WINDOW_MANAGER_HILOG_H
