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

#include "window_display_isolation_policy.h"

#include <parameters.h>

#include "string_util.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr const char* MOVE_ISOLATED_DISPLAY_IDS_PARAM_KEY =
    "persist.windowlayout.displayisolation.moveisolateddisplayids";
constexpr const char* DRAG_ISOLATED_DISPLAY_IDS_PARAM_KEY =
    "persist.windowlayout.displayisolation.dragisolateddisplayids";
} // namespace

std::optional<DisplayIsolationConfig> WindowDisplayIsolationPolicy::displayIsolationConfig_;

std::string DisplayIsolationConfig::ToString() const
{
    return "moveIsolatedDisplayIds: [" + StringUtil::JoinValueSet(moveIsolatedDisplayIds, ',') +
           "], dragIsolatedDisplayIds: [" + StringUtil::JoinValueSet(dragIsolatedDisplayIds, ',') + "]";
}

bool WindowDisplayIsolationPolicy::IsMoveEnable(DisplayId fromDisplayId, DisplayId toDisplayId)
{
    const auto config = GetDisplayIsolationConfig();
    return IsOperationEnable(config.moveIsolatedDisplayIds, fromDisplayId, toDisplayId);
}

bool WindowDisplayIsolationPolicy::IsDragEnable(DisplayId fromDisplayId, DisplayId toDisplayId)
{
    const auto config = GetDisplayIsolationConfig();
    return IsOperationEnable(config.dragIsolatedDisplayIds, fromDisplayId, toDisplayId);
}

void WindowDisplayIsolationPolicy::SaveDisplayIsolationSystemConfig(const DisplayIsolationConfig& config)
{
    const std::string moveDisplayIds = StringUtil::JoinValueSet(config.moveIsolatedDisplayIds, ',');
    const std::string dragDisplayIds = StringUtil::JoinValueSet(config.dragIsolatedDisplayIds, ',');
    system::SetParameter(MOVE_ISOLATED_DISPLAY_IDS_PARAM_KEY, moveDisplayIds);
    system::SetParameter(DRAG_ISOLATED_DISPLAY_IDS_PARAM_KEY, dragDisplayIds);
    displayIsolationConfig_ = config;
    TLOGI(WmsLogTag::WMS_LAYOUT, "config: %{public}s", config.ToString().c_str());
}

DisplayIsolationConfig WindowDisplayIsolationPolicy::LoadDisplayIsolationSystemConfig()
{
    const std::string moveDisplayIds = system::GetParameter(MOVE_ISOLATED_DISPLAY_IDS_PARAM_KEY, "");
    const std::string dragDisplayIds = system::GetParameter(DRAG_ISOLATED_DISPLAY_IDS_PARAM_KEY, "");
    DisplayIsolationConfig config;
    config.moveIsolatedDisplayIds = StringUtil::ParseValueSet<DisplayId>(moveDisplayIds, ',');
    config.dragIsolatedDisplayIds = StringUtil::ParseValueSet<DisplayId>(dragDisplayIds, ',');
    TLOGD(WmsLogTag::WMS_LAYOUT, "config: %{public}s", config.ToString().c_str());
    return config;
}

bool WindowDisplayIsolationPolicy::IsOperationEnable(const std::set<DisplayId>& isolatedDisplayIds,
                                                     DisplayId fromDisplayId,
                                                     DisplayId toDisplayId)
{
    if (fromDisplayId == DISPLAY_ID_INVALID || toDisplayId == DISPLAY_ID_INVALID || fromDisplayId == toDisplayId) {
        return true;
    }
    return isolatedDisplayIds.count(fromDisplayId) == 0 && isolatedDisplayIds.count(toDisplayId) == 0;
}

DisplayIsolationConfig WindowDisplayIsolationPolicy::GetDisplayIsolationConfig()
{
    if (!displayIsolationConfig_.has_value()) {
        displayIsolationConfig_ = LoadDisplayIsolationSystemConfig();
    }
    return displayIsolationConfig_.value();
}

} // namespace OHOS::Rosen
