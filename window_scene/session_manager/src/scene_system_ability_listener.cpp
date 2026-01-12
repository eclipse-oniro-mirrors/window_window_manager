/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "scene_system_ability_listener.h"

#ifdef RES_SCHED_ENABLE
#include "res_sched_client.h"
#include "res_type.h"
#endif

#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
#include "session_manager/include/scene_session_manager.h"
#endif

#include "system_ability_definition.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
void SceneSystemAbilityListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
#ifdef RES_SCHED_ENABLE
    if (systemAbilityId == RES_SCHED_SYS_ABILITY_ID) {
        std::unordered_map<std::string, std::string> payload {
            { "pid", info_.scbPid_ },
            { "tid", info_.scbTid_ },
            { "uid", info_.scbUid_ },
            { "bundleName", info_.scbBundleName_ },
        };
        uint32_t type = OHOS::ResourceSchedule::ResType::RES_TYPE_REPORT_SCENE_BOARD;
        OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, 0, payload);
        constexpr int32_t userInteraction = 2;
        payload = {
            { "pid", info_.scbPid_ },
            { "tid", info_.ssmTid_ },
            { "uid", info_.scbUid_ },
            { "extType", "10002" },
            { "cgroupPrio", "1" },
            { "isSa", "0" },
            { "threadName", info_.ssmThreadName_ }
        };
        type = ResourceSchedule::ResType::RES_TYPE_KEY_PERF_SCENE;
        OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, userInteraction, payload);
        TLOGI(WmsLogTag::WMS_MAIN, "set RES_TYPE_KEY_PERF_SCENE success");
    }
#endif

#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
    if (systemAbilityId == POWER_MANAGER_SERVICE_ID) {
        SceneSessionManager::GetInstance().RegisterBrightnessDataChangeListener();
    }
#endif
}

void SceneSystemAbilityListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
    if (systemAbilityId == POWER_MANAGER_SERVICE_ID) {
        SceneSessionManager::GetInstance().UnregisterBrightnessDataChangeListener();
    }
#endif
}
} // namespace Rosen
} // namespace OHOS

