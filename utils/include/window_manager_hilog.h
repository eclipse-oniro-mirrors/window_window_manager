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

#include "hilog/log.h"
#include <unordered_map>
namespace OHOS {
namespace Rosen {
static constexpr unsigned int HILOG_DOMAIN_WINDOW = 0xD004200;
static constexpr unsigned int HILOG_DOMAIN_DISPLAY = 0xD004201;

#define PRINT_WLOG(level, ...) HILOG_IMPL(LABEL.type, level, LABEL.domain, LABEL.tag, ##__VA_ARGS__)

#define WLOGD(fmt, ...) PRINT_WLOG(LOG_DEBUG, "(%{public}d)" fmt, __LINE__, ##__VA_ARGS__)
#define WLOGI(fmt, ...) PRINT_WLOG(LOG_INFO, "(%{public}d)" fmt, __LINE__, ##__VA_ARGS__)
#define WLOGW(fmt, ...) PRINT_WLOG(LOG_WARN, "(%{public}d)" fmt, __LINE__, ##__VA_ARGS__)
#define WLOGE(fmt, ...) PRINT_WLOG(LOG_ERROR, "(%{public}d)" fmt, __LINE__, ##__VA_ARGS__)

#define C_W_FUNC __func__

#define WLOGFD(fmt, ...) WLOGD("%{public}s: " fmt, C_W_FUNC, ##__VA_ARGS__)
#define WLOGFI(fmt, ...) WLOGI("%{public}s: " fmt, C_W_FUNC, ##__VA_ARGS__)
#define WLOGFW(fmt, ...) WLOGW("%{public}s: " fmt, C_W_FUNC, ##__VA_ARGS__)
#define WLOGFE(fmt, ...) WLOGE("%{public}s: " fmt, C_W_FUNC, ##__VA_ARGS__)

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
    END = 256,                 // Last one, do not use
};

inline const std::unordered_map<WmsLogTag, const char *> DOMAIN_CONTENTS_MAP = {
    { WmsLogTag::DEFAULT, "WMS" },
    { WmsLogTag::DMS, "DMS" },
    { WmsLogTag::WMS_MAIN, "WMSMain" },
    { WmsLogTag::WMS_SUB, "WMSSub" },
    { WmsLogTag::WMS_SCB, "WMSScb" },
    { WmsLogTag::WMS_DIALOG, "WMSDialog" },
    { WmsLogTag::WMS_SYSTEM, "WMSSystem" },
    { WmsLogTag::WMS_FOCUS, "WMSFocus" },
    { WmsLogTag::WMS_LAYOUT, "WMSLayout" },
    { WmsLogTag::WMS_IMMS, "WMSImms" },
    { WmsLogTag::WMS_LIFE, "WMSLife" },
    { WmsLogTag::WMS_KEYBOARD, "WMSKeyboard" },
    { WmsLogTag::WMS_EVENT, "WMSEvent" },
    { WmsLogTag::WMS_UIEXT, "WMSUiext" },
    { WmsLogTag::WMS_PIP, "WMSPiP" },
    { WmsLogTag::WMS_RECOVER, "WMSRecover" },
    { WmsLogTag::WMS_MULTI_USER, "WMSMultiUser" },
    { WmsLogTag::WMS_TOAST, "WMSToast" },
    { WmsLogTag::WMS_MULTI_WINDOW, "WMSMultiWindow" },
    { WmsLogTag::WMS_INPUT_KEY_FLOW, "InputKeyFlow" },
};
#ifdef IS_RELEASE_VERSION
#define WMS_FILE_NAME ""
#else
#define WMS_FILE_NAME (__builtin_strrchr(__FILE_NAME__, '/') ?   \
                       __builtin_strrchr(__FILE_NAME__, '/') + 1 : __FILE_NAME__)
#endif

#define FMT_PREFIX "[%{public}s] %{public}s(%{public}d): "

#define FMT_PREFIX_NO_FUNC "[%{public}s]: "

#define PRINT_TLOG(level, tag, ...)                                                                     \
    do {                                                                                                \
        uint32_t hilogDomain = HILOG_DOMAIN_WINDOW + static_cast<uint32_t>(tag);                        \
        const char *domainContent = DOMAIN_CONTENTS_MAP.count(tag) ? DOMAIN_CONTENTS_MAP.at(tag) : "";  \
        HILOG_IMPL(LOG_CORE, level, hilogDomain, domainContent, ##__VA_ARGS__);                         \
    } while (0)

#define TLOGD(tag, fmt, ...) \
PRINT_TLOG(LOG_DEBUG, tag, FMT_PREFIX fmt, WMS_FILE_NAME, C_W_FUNC, __LINE__, ##__VA_ARGS__)
#define TLOGI(tag, fmt, ...) \
PRINT_TLOG(LOG_INFO, tag, FMT_PREFIX fmt, WMS_FILE_NAME, C_W_FUNC, __LINE__, ##__VA_ARGS__)
#define TLOGW(tag, fmt, ...) \
PRINT_TLOG(LOG_WARN, tag, FMT_PREFIX fmt, WMS_FILE_NAME, C_W_FUNC, __LINE__, ##__VA_ARGS__)
#define TLOGE(tag, fmt, ...) \
PRINT_TLOG(LOG_ERROR, tag, FMT_PREFIX fmt, WMS_FILE_NAME, C_W_FUNC, __LINE__, ##__VA_ARGS__)

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

} // namespace OHOS
}
#endif // FRAMEWORKS_WM_INCLUDE_WINDOW_MANAGER_HILOG_H
