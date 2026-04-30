/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef WINDOW_HISTOGRAM_MANAGEMENT_H
#define WINDOW_HISTOGRAM_MANAGEMENT_H

#include "wm_common.h"
#include "window_manager_hilog.h"

#ifndef WINDOW_MANAGER_FEATURE_SUPPORT_API_METRICS

namespace OHOS {
namespace Rosen {

inline void HISTOGRAM_BOOLEAN(const char* name, int32_t value)
{
    TLOGD(WmsLogTag::DEFAULT, "void HISTOGRAM_BOOLEAN called: %{public}s, value=%{public}d", name, value);
    (void)name;
    (void)value;
}

inline void HISTOGRAM_ENUMERATION(const char* name, int32_t sample, int32_t boundary)
{
    TLOGD(WmsLogTag::DEFAULT, "void HISTOGRAM_ENUMERATION called: %{public}s, sample=%{public}d, boundary=%{public}d",
        name, sample, boundary);
    (void)name;
    (void)sample;
    (void)boundary;
}

inline void HISTOGRAM_CUSTOM_COUNTS(const char* name, int32_t sample,
                                    int32_t min, int32_t max, size_t bucket_count)
{
    TLOGD(WmsLogTag::DEFAULT, "void HISTOGRAM_CUSTOM_COUNTS called: %{public}s, sample=%{public}d",
        name, sample);
    (void)name;
    (void)sample;
    (void)min;
    (void)max;
    (void)bucket_count;
}

inline void HISTOGRAM_TIMES(const char* name, int32_t sample)
{
    TLOGD(WmsLogTag::DEFAULT, "void HISTOGRAM_TIMES called: %{public}s, sample=%{public}d", name, sample);
    (void)name;
    (void)sample;
}

inline void HISTOGRAM_PERCENTAGE(const char* name, int32_t sample)
{
    TLOGD(WmsLogTag::DEFAULT, "void HISTOGRAM_PERCENTAGE called: %{public}s, sample=%{public}d", name, sample);
    (void)name;
    (void)sample;
}

inline void HISTOGRAM_ENUMERATION_ERROR_CODE(const char* name, WmErrorCode errorCode)
{
    TLOGD(WmsLogTag::DEFAULT, "void HISTOGRAM_ENUMERATION_ERROR_CODE called: %{public}s, code=%{public}d",
        name, errorCode);
    (void)name;
    (void)errorCode;
}

inline void HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE(const char* name, int32_t errorCode)
{
    TLOGD(WmsLogTag::DEFAULT, "void WINDOW_MANAGER_ERROR_CODE called: %{public}s, code=%{public}d",
        name, errorCode);
    (void)name;
    (void)errorCode;
}

} // namespace Rosen
} // namespace OHOS

#else
#include "histogram_plugin_macros.h"
#include "oh_window_comm.h"

namespace OHOS {
namespace Rosen {
/**
 * @brief Base value for WM error codes
 */
constexpr int32_t WM_ERROR_BASE = 1300000;

/**
 * @brief Histogram index constants for specific error codes
 */
constexpr int32_t WM_ERROR_HISTOGRAM_INDEX_OK = 0;
constexpr int32_t WM_ERROR_HISTOGRAM_INDEX_NO_PERMISSION = 51;
constexpr int32_t WM_ERROR_HISTOGRAM_INDEX_NOT_SYSTEM_APP = 52;
constexpr int32_t WM_ERROR_HISTOGRAM_INDEX_INVALID_PARAM = 53;
constexpr int32_t WM_ERROR_HISTOGRAM_INDEX_DEVICE_NOT_SUPPORT = 54;

/**
 * @brief Histogram index constants for WindowManager_ErrorCode
 */
constexpr int32_t WM_MANAGER_ERROR_HISTOGRAM_INDEX_OK = 0;
constexpr int32_t WM_MANAGER_ERROR_HISTOGRAM_INDEX_NO_PERMISSION = 51;
constexpr int32_t WM_MANAGER_ERROR_HISTOGRAM_INDEX_INVALID_PARAM = 52;
constexpr int32_t WM_MANAGER_ERROR_HISTOGRAM_INDEX_DEVICE_NOT_SUPPORTED = 53;
constexpr int32_t WM_MANAGER_ERROR_HISTOGRAM_INDEX_INVAILD_WINDOW_ID = 54;
constexpr int32_t WM_MANAGER_ERROR_HISTOGRAM_INDEX_SERVICE_ERROR = 55;

/**
 * @brief Maximum value for WM error code histogram
 * @note The maximum index is 55 (mapped from WM_MANAGER_ERROR_HISTOGRAM_INDEX_SERVICE_ERROR)
 */
constexpr int32_t WM_ERROR_HISTOGRAM_MAX = 55;

/**
 * @brief Convert WmErrorCode to histogram index
 * @param error - WmErrorCode enum value
 * @return Index value:
 *         - WM_OK -> 0
 *         - WM_ERROR_NO_PERMISSION (201) -> 51
 *         - WM_ERROR_NOT_SYSTEM_APP (202) -> 52
 *         - WM_ERROR_INVALID_PARAM (401) -> 53
 *         - WM_ERROR_DEVICE_NOT_SUPPORT (801) -> 54
 *         - values >= WM_ERROR_BASE (1300000) -> value - WM_ERROR_BASE (1, 2, 3, ...)
 * @note constexpr function enables compile-time evaluation when error is a compile-time constant
 */
constexpr int32_t WmErrorCodeToIndex(WmErrorCode error)
{
    switch (error) {
        case WmErrorCode::WM_ERROR_NO_PERMISSION:
            return WM_ERROR_HISTOGRAM_INDEX_NO_PERMISSION;
        case WmErrorCode::WM_ERROR_NOT_SYSTEM_APP:
            return WM_ERROR_HISTOGRAM_INDEX_NOT_SYSTEM_APP;
        case WmErrorCode::WM_ERROR_INVALID_PARAM:
            return WM_ERROR_HISTOGRAM_INDEX_INVALID_PARAM;
        case WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT:
            return WM_ERROR_HISTOGRAM_INDEX_DEVICE_NOT_SUPPORT;
        default: {
            int32_t value = static_cast<int32_t>(error);
            return (value >= WM_ERROR_BASE) ? (value - WM_ERROR_BASE) : WM_ERROR_HISTOGRAM_INDEX_OK;
        }
    }
}

/**
 * @brief Histogram enumeration macro for WmErrorCode
 * @param name - The histogram name (string)
 * @param errorCode - WmErrorCode enum value, will be converted to index via WmErrorCodeToIndex()
 * @note Compile-time type check ensures errorCode is WmErrorCode type
 */
#define HISTOGRAM_ENUMERATION_ERROR_CODE(name, errorCode) \
    TLOGD(WmsLogTag::DEFAULT, "HISTOGRAM_ENUMERATION_ERROR_CODE called: %{public}s", name); \
    static_assert(std::is_same_v<std::decay_t<decltype(errorCode)>, WmErrorCode>, \
        "HISTOGRAM_ENUMERATION_ERROR_CODE: errorCode parameter must be WmErrorCode type"); \
    HISTOGRAM_ENUMERATION(name, WmErrorCodeToIndex(errorCode), WM_ERROR_HISTOGRAM_MAX)

/**
 * @brief Convert WindowManager_ErrorCode to histogram index
 * @param error - WindowManager_ErrorCode enum value
 * @return Index value:
 *         - OK -> 0
 *         - NO_PERMISSION (201) -> 51
 *         - INVALID_PARAM (401) -> 52
 *         - DEVICE_NOT_SUPPORTED (801) -> 53
 *         - INVAILD_WINDOW_ID (1000) -> 54
 *         - SERVICE_ERROR (2000) -> 55
 *         - values >= WM_ERROR_BASE (1300000) -> value - WM_ERROR_BASE (1, 2, 3, ...)
 * @note constexpr function enables compile-time evaluation when error is a compile-time constant
 */
constexpr int32_t WindowManagerErrorCodeToIndex(WindowManager_ErrorCode error)
{
    switch (error) {
        case WindowManager_ErrorCode::OK:
            return WM_MANAGER_ERROR_HISTOGRAM_INDEX_OK;
        case WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_NO_PERMISSION:
            return WM_MANAGER_ERROR_HISTOGRAM_INDEX_NO_PERMISSION;
        case WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM:
            return WM_MANAGER_ERROR_HISTOGRAM_INDEX_INVALID_PARAM;
        case WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED:
            return WM_MANAGER_ERROR_HISTOGRAM_INDEX_DEVICE_NOT_SUPPORTED;
        case WindowManager_ErrorCode::INVAILD_WINDOW_ID:
            return WM_MANAGER_ERROR_HISTOGRAM_INDEX_INVAILD_WINDOW_ID;
        case WindowManager_ErrorCode::SERVICE_ERROR:
            return WM_MANAGER_ERROR_HISTOGRAM_INDEX_SERVICE_ERROR;
        default: {
            int32_t value = static_cast<int32_t>(error);
            return (value >= WM_ERROR_BASE) ? (value - WM_ERROR_BASE) : WM_MANAGER_ERROR_HISTOGRAM_INDEX_OK;
        }
    }
}

/**
 * @brief Histogram enumeration macro for WindowManager_ErrorCode
 * @param name - The histogram name (string)
 * @param errorCode - WindowManager_ErrorCode enum value, will be converted to index via WindowManagerErrorCodeToIndex()
 * @note Compile-time type check ensures errorCode is WindowManager_ErrorCode type
 */
#define HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE(name, errorCode) \
    TLOGD(WmsLogTag::DEFAULT, "HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE called: %{public}s", name); \
    static_assert(std::is_same_v<std::decay_t<decltype(errorCode)>, WindowManager_ErrorCode>, \
        "HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE: errorCode parameter must be WindowManager_ErrorCode type"); \
    HISTOGRAM_ENUMERATION(name, WindowManagerErrorCodeToIndex(errorCode), WM_ERROR_HISTOGRAM_MAX)

} // namespace Rosen
} // namespace OHOS
#endif // WINDOW_MANAGER_FEATURE_SUPPORT_API_METRICS

#endif // WINDOW_HISTOGRAM_MANAGEMENT_H