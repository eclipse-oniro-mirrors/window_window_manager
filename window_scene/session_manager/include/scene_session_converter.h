/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_SESSION_MANAGER_SCENE_SESSION_CONVERTER_H
#define OHOS_SESSION_MANAGER_SCENE_SESSION_CONVERTER_H

#include <string>
#include <vector>

#include "mission_info.h"
#include "session/host/include/scene_session.h"
#include "ws_common.h"

namespace OHOS {
namespace Rosen {
class SceneSessionConverter {
public:
    static WSError ConvertToMissionInfos(std::vector<sptr<SceneSession>>& sceneSessionInfos,
        std::vector<AAFwk::MissionInfo>& missionInfos);
    static WSError ConvertToMissionInfo(const sptr<SceneSession>& sceneSession, AAFwk::MissionInfo& missionInfo);
    static WSError ConvertToMissionInfo(const sptr<SceneSession>& sceneSession, AAFwk::MissionInfo& missionInfo,
        AAFwk::DisplayInfo& displayInfo);
};
}
}
#endif // OHOS_SESSION_MANAGER_SCENE_SESSION_CONVERTER_H
