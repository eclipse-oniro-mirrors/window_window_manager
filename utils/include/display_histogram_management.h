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

#ifndef DISPLAY_HISTOGRAM_MANAGEMENT_H
#define DISPLAY_HISTOGRAM_MANAGEMENT_H

#include "dm_common.h"
#include "oh_display_info.h"

#ifndef WINDOW_MANAGER_FEATURE_SUPPORT_API_METRICS

namespace OHOS {
namespace Rosen {

inline void HISTOGRAM_BOOLEAN(const char* name, int32_t value)
{
    (void)name;
    (void)value;
}

inline void HISTOGRAM_ENUMERATION(const char* name, int32_t sample, int32_t boundary)
{
    (void)name;
    (void)sample;
    (void)boundary;
}

inline void HISTOGRAM_CUSTOM_COUNTS(const char* name, int32_t sample,
                                    int32_t min, int32_t max, size_t bucket_count)
{
    (void)name;
    (void)sample;
    (void)min;
    (void)max;
    (void)bucket_count;
}

inline void HISTOGRAM_TIMES(const char* name, int32_t sample)
{
    (void)name;
    (void)sample;
}

inline void HISTOGRAM_PERCENTAGE(const char* name, int32_t sample)
{
    (void)name;
    (void)sample;
}

inline void HISTOGRAM_ENUMERATION_DM_ERROR_CODE(const char* name, DmErrorCode errorCode)
{
    (void)name;
    (void)errorCode;
}

inline void HISTOGRAM_ENUMERATION_C_ERROR_CODE(const char* name, NativeDisplayManager_ErrorCode errorCode)
{
    (void)name;
    (void)errorCode;
}

} // namespace Rosen
} // namespace OHOS

#else
#include "histogram_plugin_macros.h"

namespace OHOS {
namespace Rosen {
/**
 * @brief Base value for DM error codes
 */
constexpr int32_t DM_ERROR_BASE = 1400000;

/**
 * @brief Histogram index constants for specific error codes
 */
constexpr int32_t DM_ERROR_HISTOGRAM_INDEX_OK = 0;
constexpr int32_t DM_ERROR_HISTOGRAM_INDEX_NO_PERMISSION = 5;
constexpr int32_t DM_ERROR_HISTOGRAM_INDEX_NOT_SYSTEM_APP = 6;
constexpr int32_t DM_ERROR_HISTOGRAM_INDEX_INVALID_PARAM = 7;
constexpr int32_t DM_ERROR_HISTOGRAM_INDEX_DEVICE_NOT_SUPPORT = 8;

/**
 * @brief Maximum value for DM error code histogram
 * @note The maximum index is 8 (mapped from DM_ERROR_HISTOGRAM_INDEX_DEVICE_NOT_SUPPORT)
 */
constexpr int32_t DM_ERROR_HISTOGRAM_MAX = 8;

/**
 * @brief Convert DmErrorCode to histogram index
 * @param error - DmErrorCode enum value
 * @return Index value:
 *         - DM_OK -> 0
 *         - DM_ERROR_NO_PERMISSION (201) -> 5
 *         - DM_ERROR_NOT_SYSTEM_APP (202) -> 6
 *         - DM_ERROR_INVALID_PARAM (401) -> 7
 *         - DM_ERROR_DEVICE_NOT_SUPPORT (801) -> 8
 *         - values >= DM_ERROR_BASE (1400000) -> value - DM_ERROR_BASE (1, 2, 3, 4)
 * @note constexpr function enables compile-time evaluation when error is a compile-time constant
 */
constexpr int32_t DmErrorCodeToIndex(DmErrorCode error)
{
    switch (error) {
        case DmErrorCode::DM_ERROR_NO_PERMISSION:
            return DM_ERROR_HISTOGRAM_INDEX_NO_PERMISSION;
        case DmErrorCode::DM_ERROR_NOT_SYSTEM_APP:
            return DM_ERROR_HISTOGRAM_INDEX_NOT_SYSTEM_APP;
        case DmErrorCode::DM_ERROR_INVALID_PARAM:
            return DM_ERROR_HISTOGRAM_INDEX_INVALID_PARAM;
        case DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT:
            return DM_ERROR_HISTOGRAM_INDEX_DEVICE_NOT_SUPPORT;
        default: {
            int32_t value = static_cast<int32_t>(error);
            return (value >= DM_ERROR_BASE) ? (value - DM_ERROR_BASE) : DM_ERROR_HISTOGRAM_INDEX_OK;
        }
    }
}

/**
 * @brief Histogram enumeration macro for DmErrorCode
 * @param name - The histogram name (string)
 * @param errorCode - DmErrorCode enum value, will be converted to index via DmErrorCodeToIndex()
 * @note Compile-time type check ensures errorCode is DmErrorCode type
 */
#define HISTOGRAM_ENUMERATION_DM_ERROR_CODE(name, errorCode) \
    static_assert(std::is_same_v<decltype(errorCode), DmErrorCode>, \
        "HISTOGRAM_ENUMERATION_DM_ERROR_CODE: errorCode parameter must be DmErrorCode type"); \
    HISTOGRAM_ENUMERATION(name, DmErrorCodeToIndex(errorCode), DM_ERROR_HISTOGRAM_MAX)

/**
 * @brief Convert NativeDisplayManager_ErrorCode to histogram index
 * @param error - NativeDisplayManager_ErrorCode enum value
 * @return Index value (same mapping as DmErrorCode since values are identical)
 */
constexpr int32_t NdkErrorCodeToIndex(NativeDisplayManager_ErrorCode error)
{
    switch (error) {
        case NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_NO_PERMISSION:
            return DM_ERROR_HISTOGRAM_INDEX_NO_PERMISSION;
        case NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_NOT_SYSTEM_APP:
            return DM_ERROR_HISTOGRAM_INDEX_NOT_SYSTEM_APP;
        case NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM:
            return DM_ERROR_HISTOGRAM_INDEX_INVALID_PARAM;
        case NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORTED:
            return DM_ERROR_HISTOGRAM_INDEX_DEVICE_NOT_SUPPORT;
        default: {
            int32_t value = static_cast<int32_t>(error);
            return (value >= DM_ERROR_BASE) ? (value - DM_ERROR_BASE) : DM_ERROR_HISTOGRAM_INDEX_OK;
        }
    }
}

/**
 * @brief Histogram enumeration macro for C API error codes
 * @param name - The histogram name (string)
 * @param errorCode - NativeDisplayManager_ErrorCode enum value
 */
#define HISTOGRAM_ENUMERATION_C_ERROR_CODE(name, errorCode) \
    static_assert(std::is_same_v<decltype(errorCode), NativeDisplayManager_ErrorCode>, \
        "HISTOGRAM_ENUMERATION_C_ERROR_CODE: errorCode parameter must be NativeDisplayManager_ErrorCode type"); \
    HISTOGRAM_ENUMERATION(name, NdkErrorCodeToIndex(errorCode), DM_ERROR_HISTOGRAM_MAX)

} // namespace Rosen
} // namespace OHOS
#endif // WINDOW_MANAGER_FEATURE_SUPPORT_API_METRICS

#endif // DISPLAY_HISTOGRAM_MANAGEMENT_H
