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

#include "scene_session_converter.h"
#include "ability_info.h"
#include "display_manager.h"

using namespace std;
namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SceneSessionConverter" };
}

WSError SceneSessionConverter::ConvertToMissionInfos(std::vector<sptr<SceneSession>>& sceneSessionInfos,
                                                     std::vector<AAFwk::MissionInfo>& missionInfos)
{
    if (sceneSessionInfos.empty()) {
        return WSError::WS_OK;
    }
    for (auto iter = sceneSessionInfos.begin(); iter != sceneSessionInfos.end(); iter++) {
        AAFwk::MissionInfo missionInfo;
        missionInfo.id = (*iter)->GetPersistentId();
        missionInfo.runningState = (*iter)->IsSessionValid() ? 0 : -1;
        missionInfo.lockedState = (*iter)->GetSessionInfo().lockedState;
        if ((*iter)->GetSessionInfo().abilityInfo != nullptr) {
            missionInfo.label = ((*iter)->GetSessionInfo().abilityInfo)->label;
            missionInfo.iconPath = ((*iter)->GetSessionInfo().abilityInfo)->iconPath;
            missionInfo.continuable = ((*iter)->GetSessionInfo().abilityInfo)->continuable;
            missionInfo.unclearable = ((*iter)->GetSessionInfo().abilityInfo)->unclearableMission;
        } else {
            WLOGFE("abilityInfo in SceneSession is nullptr, id: %{public}d", (*iter)->GetPersistentId());
        }
        if ((*iter)->GetSessionInfo().want != nullptr) {
            missionInfo.want = *((*iter)->GetSessionInfo().want);
        } else {
            WLOGFE("want in SceneSession is nullptr, id: %{public}d", (*iter)->GetPersistentId());
        }
        missionInfo.time = (*iter)->GetSessionInfo().time;
        missionInfo.continueState = (AAFwk::ContinueState)(AAFwk::ContinueState::CONTINUESTATE_UNKNOWN +
            ((*iter)->GetSessionInfo().continueState - Rosen::ContinueState::CONTINUESTATE_UNKNOWN));
        missionInfos.push_back(std::move(missionInfo));
    }
    return WSError::WS_OK;
}

WSError SceneSessionConverter::ConvertToMissionInfo(const sptr<SceneSession>& sceneSession,
                                                    AAFwk::MissionInfo& missionInfo)
{
    if (sceneSession == nullptr) {
        return WSError::WS_OK;
    }
    missionInfo.id = sceneSession->GetPersistentId();
    missionInfo.runningState = sceneSession->IsSessionValid() ? 0 : -1;
    missionInfo.lockedState = sceneSession->GetSessionInfo().lockedState;
    if (sceneSession->GetSessionInfo().abilityInfo != nullptr) {
        missionInfo.label = (sceneSession->GetSessionInfo().abilityInfo)->label;
        missionInfo.iconPath = (sceneSession->GetSessionInfo().abilityInfo)->iconPath;
        missionInfo.continuable = (sceneSession->GetSessionInfo().abilityInfo)->continuable;
        missionInfo.unclearable = (sceneSession->GetSessionInfo().abilityInfo)->unclearableMission;
    } else {
        WLOGFE("abilityInfo in SceneSession is nullptr, id: %{public}d", sceneSession->GetPersistentId());
    }
    if (sceneSession->GetSessionInfo().want != nullptr) {
        missionInfo.want = *(sceneSession->GetSessionInfo().want);
    } else {
        WLOGFE("want in SceneSession is nullptr, id: %{public}d", sceneSession->GetPersistentId());
    }
    missionInfo.time = sceneSession->GetSessionInfo().time;
    missionInfo.continueState = (AAFwk::ContinueState) (AAFwk::ContinueState::CONTINUESTATE_UNKNOWN +
        (sceneSession->GetSessionInfo().continueState - Rosen::ContinueState::CONTINUESTATE_UNKNOWN));
    return WSError::WS_OK;
}
WSError SceneSessionConverter::ConvertToMissionInfo(const sptr<SceneSession>& sceneSession,
                                                    AAFwk::MissionInfo& missionInfo,
                                                    AAFwk::DisplayInfo& displayInfo)
{
    WSError ret = ConvertToMissionInfo(sceneSession, missionInfo);
    if (ret != WSError::WS_OK || sceneSession == nullptr) {
        return ret;
    }
    displayInfo.id = static_cast<int32_t>(sceneSession->GetSessionProperty()->GetDisplayId());
    auto display = DisplayManager::GetInstance().GetDisplayById(displayInfo.id);
    if (display != nullptr) {
        displayInfo.displayName = display->GetName();
    }
    return WSError::WS_OK;
}
}
}
