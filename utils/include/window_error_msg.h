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

#ifndef WINDOW_WINDOW_MANAGER_WINDOW_ERROR_MSG_H
#define WINDOW_WINDOW_MANAGER_WINDOW_ERROR_MSG_H

#include <string>

#include "dm_common.h"
#include "wm_common.h"

namespace OHOS::Rosen {
class WindowErrorMsg {
public:
    /**
     * @brief Gets the full API-facing error message for the specified WMError.
     *
     * @param errorCode Window Manager API error code.
     * @return Full error message if the code is found; empty string otherwise.
     */
    static std::string GetErrorMsg(WMError error);

    /**
     * @brief Gets the brief API-facing error message for the specified WmErrorCode.
     *
     * @param errorCode Window Manager API error code.
     * @return Brief error message if found; empty string otherwise.
     */
    static std::string GetShortErrorMsg(WmErrorCode errorCode);

    /**
     * @brief Gets the full API-facing error message for the specified WmErrorCode.
     *
     * @param errorCode Window Manager API error code.
     * @return Full error message if found; empty string otherwise.
     */
    static std::string GetFullErrorMsg(WmErrorCode errorCode);

    /**
     * @brief Builds an API-facing Window Manager error message.
     *
     * If customMessage is empty, returns the full error message. Otherwise, appends
     * customMessage after the brief error summary.
     *
     * @param errorCode Window Manager API error code.
     * @param customMessage Additional error detail.
     * @return Built error message if the code is found; empty string otherwise.
     */
    static std::string BuildErrorMsg(WmErrorCode errorCode, const std::string& customMessage);

    /**
     * @brief Gets the full API-facing error message for the specified DMError.
     *
     * @param errorCode Display Manager API error code.
     * @return Full error message if the code is found; empty string otherwise.
     */
    static std::string GetErrorMsg(DMError error);

    /**
     * @brief Gets the brief API-facing error message for the specified DmErrorCode.
     *
     * @param errorCode Display Manager API error code.
     * @return Brief error message if found; empty string otherwise.
     */
    static std::string GetShortErrorMsg(DmErrorCode errorCode);

    /**
     * @brief Gets the full API-facing error message for the specified DmErrorCode.
     *
     * @param errorCode Display Manager API error code.
     * @return Full error message if found; empty string otherwise.
     */
    static std::string GetFullErrorMsg(DmErrorCode errorCode);

    /**
     * @brief Builds an API-facing Display Manager error message.
     *
     * If customMessage is empty, returns the full error message. Otherwise, appends
     * customMessage after the brief error summary.
     *
     * @param errorCode Display Manager API error code.
     * @param customMessage Additional error detail.
     * @return Built error message if the code is found; empty string otherwise.
     */
    static std::string BuildErrorMsg(DmErrorCode errorCode, const std::string& customMessage);
};
} // namespace OHOS::Rosen

#endif // WINDOW_WINDOW_MANAGER_WINDOW_ERROR_MSG_H
