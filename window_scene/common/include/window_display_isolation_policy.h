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

#ifndef OHOS_ROSEN_WINDOW_DISPLAY_ISOLATION_POLICY_H
#define OHOS_ROSEN_WINDOW_DISPLAY_ISOLATION_POLICY_H

#include <optional>
#include <set>
#include <string>

#include "dm_common.h"

namespace OHOS::Rosen {

/**
 * @brief Configuration of display isolation for window operations.
 */
struct DisplayIsolationConfig {
    /**
     * @brief Isolated Display IDs for window move operations.
     */
    std::set<DisplayId> moveIsolatedDisplayIds;

    /**
     * @brief Isolated Display IDs for window drag operations.
     */
    std::set<DisplayId> dragIsolatedDisplayIds;

    /**
     * @brief Converts the display isolation configuration to a log-friendly string.
     *
     * @return A string containing all isolated display IDs.
     */
    std::string ToString() const;
};

/**
 * @brief Provides display isolation policy checks for window operations.
 */
class WindowDisplayIsolationPolicy final {
public:
    WindowDisplayIsolationPolicy() = delete;
    ~WindowDisplayIsolationPolicy() = delete;

    /**
     * @brief Checks whether a window move is enabled between two displays.
     *
     * @param fromDisplayId Display where the window is currently located.
     * @param toDisplayId Display requested as the move destination.
     * @return True if the move is enabled; false if it is blocked by display isolation.
     */
    static bool IsMoveEnable(DisplayId fromDisplayId, DisplayId toDisplayId);

    /**
     * @brief Checks whether a window drag is enabled between two displays.
     *
     * @param fromDisplayId Display where the drag operation started.
     * @param toDisplayId Candidate display for the drag operation.
     * @return True if the drag is enabled; false if it is blocked by display isolation.
     */
    static bool IsDragEnable(DisplayId fromDisplayId, DisplayId toDisplayId);

    /**
     * @brief Persists the configuration and updates the current process cache.
     *
     * @param config Display isolation configuration to persist.
     */
    static void SaveDisplayIsolationSystemConfig(const DisplayIsolationConfig& config);

    /**
     * @brief Loads the display isolation configuration from system parameters.
     *
     * @return The parsed display isolation configuration.
     */
    static DisplayIsolationConfig LoadDisplayIsolationSystemConfig();

private:
    /**
     * @brief Checks whether an operation is enabled for the specified isolated display set.
     *
     * Invalid display IDs and movement within the same display are treated as enabled.
     *
     * @param isolatedDisplayIds Displays isolated from the operation.
     * @param fromDisplayId Display where the operation starts.
     * @param toDisplayId Display where the operation ends.
     * @return True if the operation is enabled; false otherwise.
     */
    static bool IsOperationEnable(const std::set<DisplayId>& isolatedDisplayIds,
                                  DisplayId fromDisplayId,
                                  DisplayId toDisplayId);

    /**
     * @brief Gets the display isolation configuration snapshot.
     *
     * The snapshot is initialized from system parameters on first access.
     *
     * @return A copy of the cached display isolation configuration.
     */
    static DisplayIsolationConfig GetDisplayIsolationConfig();

    /**
     * @brief Cached display isolation configuration for the current process.
     */
    static std::optional<DisplayIsolationConfig> displayIsolationConfig_;
};

} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_DISPLAY_ISOLATION_POLICY_H
