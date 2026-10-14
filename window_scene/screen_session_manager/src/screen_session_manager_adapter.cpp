/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "screen_session_manager_adapter.h"
#include "dms_global_mutex.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "window_manager_hilog.h"
#include "fold_screen_state_internel.h"

namespace OHOS {
namespace Rosen {

WM_IMPLEMENT_SINGLE_INSTANCE(ScreenSessionManagerAdapter)
#define INIT_PROXY_CHECK_RETURN() FREE_GLOBAL_LOCK_FOR_IPC()

DMError ScreenSessionManagerAdapter::RegisterDisplayManagerAgent(
    const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
{
    return dmAgentContainer_.RegisterAgent(displayManagerAgent, type) ? DMError::DM_OK : DMError::DM_ERROR_NULLPTR;
}

DMError ScreenSessionManagerAdapter::UnregisterDisplayManagerAgent(
    const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
{
    return dmAgentContainer_.UnregisterAgent(displayManagerAgent, type) ? DMError::DM_OK : DMError::DM_ERROR_NULLPTR;
}

void ScreenSessionManagerAdapter::OnDisplayChange(sptr<DisplayInfo> displayInfo, DisplayChangeEvent event, int32_t uid)
{
    INIT_PROXY_CHECK_RETURN();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "Invalid display info");
        return;
    }

    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agents is empty");
        return;
    }
    for (const auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (ScreenSessionManager::GetInstance().GetStoredPidFromUid(uid, agentPid)) {
            TLOGE(WmsLogTag::DMS, "no notify");
            continue;
        }
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER)) {
            agent->OnDisplayChange(displayInfo, event);
            TLOGE(WmsLogTag::DMS, "notify, pid: %{public}d", agentPid);
        }
    }
}

void ScreenSessionManagerAdapter::OnDisplayChange(sptr<DisplayInfo> displayInfo, DisplayChangeEvent event)
{
    INIT_PROXY_CHECK_RETURN();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "Invalid display info");
        return;
    }

    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "event:%{public}d, displayId:%{public}" PRIu64 "OnDisplayChange agent is null",
            event, displayInfo->GetDisplayId());
        return;
    }
    std::ostringstream notifiedPids;
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER)) {
            agent->OnDisplayChange(displayInfo, event);
            notifiedPids << ' ' << agentPid;
        }
    }
    if (event == DisplayChangeEvent::UPDATE_REFRESHRATE) {
        TLOGD(WmsLogTag::DMS,
            "event:%{public}d, displayId:%{public}" PRIu64 ", agent size: %{public}u, notified: [%{public}s]",
            event, displayInfo->GetDisplayId(), static_cast<uint32_t>(agents.size()), notifiedPids.str().c_str());
    } else {
        TLOGNI(WmsLogTag::DMS,
            "event:%{public}d, displayId:%{public}" PRIu64 ", agent size: %{public}u, notified: [%{public}s]",
            event, displayInfo->GetDisplayId(), static_cast<uint32_t>(agents.size()), notifiedPids.str().c_str());
    }
}

void ScreenSessionManagerAdapter::OnDisplayAttributeChange(sptr<DisplayInfo> displayInfo,
    const std::vector<std::string>& attributes, int32_t uid)
{
    if (uid != INVALID_UID) {
        TLOGD(WmsLogTag::DMS, "Notify display attribute change with uid");
    }
    INIT_PROXY_CHECK_RETURN();
    if (attributes.empty()) {
        TLOGE(WmsLogTag::DMS, "attributes is empty");
        return;
    }
 
    std::set<sptr<IDisplayManagerAgent>> agents;
    dmAttributeAgentContainer_.ParseAttributeAgentsMap([&](auto& map) {
        for (auto it = map.begin(); it != map.end(); ++it) {
            if (IsAgentListenedAttributes(it->second.second, attributes)) {
                agents.insert(it->second.first);
            }
        }
        });

    for (auto& agent : agents) {
        int32_t agentPid = dmAttributeAgentContainer_.GetAgentPid(agent);
        if (uid != INVALID_UID && ScreenSessionManager::GetInstance().GetStoredPidFromUid(uid, agentPid)) {
            TLOGI(WmsLogTag::DMS, "Is not current uid, do not need to notify");
            continue;
        }
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid,
            DisplayManagerAgentType::DISPLAY_ATTRIBUTE_CHANGED_LISTENER)) {
            agent->OnDisplayAttributeChange(displayInfo, attributes);
        }
    }
}

bool ScreenSessionManagerAdapter::IsAgentListenedAttributes(std::set<std::string>& listenedAttributes,
    const std::vector<std::string>& attributes)
{
    for (auto attribute : attributes) {
        if (listenedAttributes.find(attribute) != listenedAttributes.end()) {
            return true;
        }
    }
    return false;
}

void ScreenSessionManagerAdapter::NotifyScreenModeChange(const std::vector<sptr<ScreenInfo>> screenInfos)
{
    INIT_PROXY_CHECK_RETURN();
    if (screenInfos.empty()) {
        TLOGE(WmsLogTag::DMS, "Empty screen info vector received");
        return;
    }

    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_MODE_CHANGE_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid,
            DisplayManagerAgentType::SCREEN_MODE_CHANGE_EVENT_LISTENER)) {
            agent->NotifyScreenModeChange(screenInfos);
        }
    }
}

void ScreenSessionManagerAdapter::OnScreenChange(sptr<ScreenInfo> screenInfo, ScreenChangeEvent event)
{
    INIT_PROXY_CHECK_RETURN();
    if (screenInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "error, screenInfo is nullptr.");
        return;
    }

    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "screenId:%{public}" PRIu64 ", agent is null", screenInfo->GetScreenId());
        return;
    }
    std::ostringstream notifiedPids;
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid, DisplayManagerAgentType::SCREEN_EVENT_LISTENER)) {
            agent->OnScreenChange(screenInfo, event);
            notifiedPids << ' ' << agentPid;
        }
    }
    TLOGNI(WmsLogTag::DMS, "screenId:%{public}" PRIu64 ", agent size: %{public}u, notified: [%{public}s]",
        screenInfo->GetScreenId(), static_cast<uint32_t>(agents.size()), notifiedPids.str().c_str());
}

bool ScreenSessionManagerAdapter::NotifyDisplayPowerEvent(DisplayPowerEvent event, EventStatus status)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return false;
    }
    TLOGD(WmsLogTag::DMS, "Received Display Power Event: %{public}d", static_cast<int>(event));
    for (auto& agent : agents) {
        agent->NotifyDisplayPowerEvent(event, status);
    }
    return true;
}

bool ScreenSessionManagerAdapter::NotifySpecifiedDisplayPowerEvent(DisplayId displayId,
    DisplayPowerEvent event, EventStatus status)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return false;
    }
    TLOGD(WmsLogTag::DMS, "Received Display Power Event: %{public}d", static_cast<int>(event));
    for (auto& agent : agents) {
        agent->NotifySpecifiedDisplayPowerEvent(displayId, event, status);
    }
    return true;
}

bool ScreenSessionManagerAdapter::NotifyDisplayStateChanged(DisplayId id, DisplayState state)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_STATE_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return false;
    }
    TLOGNFI(WmsLogTag::DMS,
        "Display State Changed: ID=%{public}d State=%{public}d",
        static_cast<uint32_t>(id), static_cast<uint32_t>(state));
    for (auto& agent : agents) {
        agent->NotifyDisplayStateChanged(id, state);
    }
    return true;
}

bool ScreenSessionManagerAdapter::NotifyDisplayStateChangedById(DisplayId displayId, DisplayState state)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_STATE_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return false;
    }
    TLOGNFI(WmsLogTag::DMS,
        "Display State Changed: ID=%{public}d State=%{public}d",
        static_cast<uint32_t>(displayId), static_cast<uint32_t>(state));
    for (auto& agent : agents) {
        agent->NotifyDisplayStateChangedById(displayId, state);
    }
    return true;
}

void ScreenSessionManagerAdapter::OnDisplayCreate(sptr<DisplayInfo> displayInfo)
{
    INIT_PROXY_CHECK_RETURN();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayInfo nullptr");
        return;
    }
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    TLOGNFI(WmsLogTag::DMS, "start, agent size: %{public}u", static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER)) {
            agent->OnDisplayCreate(displayInfo);
        }
    }
}

void ScreenSessionManagerAdapter::OnDisplayDestroy(DisplayId displayId)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    TLOGNFI(WmsLogTag::DMS, "agent size: %{public}u", static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER)) {
            agent->OnDisplayDestroy(displayId);
        }
    }
}

void ScreenSessionManagerAdapter::NotifyPrivateWindowStateChanged(bool hasPrivate)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::PRIVATE_WINDOW_LISTENER);
    if (agents.empty()) {
        TLOGE_LIMITN_MIN(WmsLogTag::DMS, THREE_TIMES, "agent is null");
        return;
    }

    TLOGNFI(WmsLogTag::DMS, "Private Window Status: %s", hasPrivate ? "Active" : "Inactive");
    for (auto& agent : agents) {
        agent->NotifyPrivateWindowStateChanged(hasPrivate);
    }
}

void ScreenSessionManagerAdapter::NotifyPrivateStateWindowListChanged(DisplayId id,
                                                                      std::vector<std::string> privacyWindowList)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::PRIVATE_WINDOW_LIST_LISTENER);
    if (agents.empty()) {
        TLOGE_LIMITN_MIN(WmsLogTag::DMS, THREE_TIMES, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        agent->NotifyPrivateStateWindowListChanged(id, privacyWindowList);
    }
}

void ScreenSessionManagerAdapter::OnScreenGroupChange(const std::string& trigger,
                                                      const std::vector<sptr<ScreenInfo>>& screens,
                                                      ScreenGroupChangeEvent event)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    std::vector<sptr<ScreenInfo>> infos;
    for (auto& screenInfo : screens) {
        if (screenInfo != nullptr) {
            infos.emplace_back(screenInfo);
        }
    }
    if (agents.empty() || infos.empty()) {
        TLOGE(WmsLogTag::DMS, "agent or infos is empty");
        return;
    }
    for (auto& agent : agents) {
        agent->OnScreenGroupChange(trigger, infos, event);
    }
}

void ScreenSessionManagerAdapter::OnScreenConnect(const sptr<ScreenInfo> screenInfo)
{
    INIT_PROXY_CHECK_RETURN();
    if (screenInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenInfo nullptr");
        return;
    }
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    TLOGNFI(WmsLogTag::DMS, "start");
    for (auto& agent : agents) {
        agent->OnScreenConnect(screenInfo);
    }
}

void ScreenSessionManagerAdapter::OnScreenDisconnect(ScreenId screenId)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    TLOGNFI(WmsLogTag::DMS, "start");
    for (auto& agent : agents) {
        agent->OnScreenDisconnect(screenId);
    }
}

void ScreenSessionManagerAdapter::NotifyFoldAngleChanged(std::vector<float> foldAngles)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::FOLD_ANGLE_CHANGED_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid,
                                                           DisplayManagerAgentType::FOLD_ANGLE_CHANGED_LISTENER)) {
            agent->NotifyFoldAngleChanged(foldAngles);
        }
    }
}

void ScreenSessionManagerAdapter::NotifyCaptureStatusChanged()
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::CAPTURE_STATUS_CHANGED_LISTENER);
    bool isCapture = ScreenSessionManager::GetInstance().IsCaptured();
    ScreenSessionManager::GetInstance().isScreenShot_ = false;
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid,
                                                           DisplayManagerAgentType::CAPTURE_STATUS_CHANGED_LISTENER)) {
            agent->NotifyCaptureStatusChanged(isCapture);
        }
    }
}

void ScreenSessionManagerAdapter::NotifyCaptureStatusChanged(bool isCapture)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::CAPTURE_STATUS_CHANGED_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid,
                                                           DisplayManagerAgentType::CAPTURE_STATUS_CHANGED_LISTENER)) {
            agent->NotifyCaptureStatusChanged(isCapture);
        }
    }
}

void ScreenSessionManagerAdapter::NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_UPDATE_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid,
                                                           DisplayManagerAgentType::DISPLAY_UPDATE_LISTENER)) {
            agent->NotifyDisplayChangeInfoChanged(info);
        }
    }
}

void ScreenSessionManagerAdapter::NotifyDisplayModeChanged(FoldDisplayMode displayMode)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_MODE_CHANGED_LISTENER);
    TLOGNFI(WmsLogTag::DMS,
        "DisplayMode:%{public}d, agent size: %{public}u", displayMode, static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid,
                                                           DisplayManagerAgentType::DISPLAY_MODE_CHANGED_LISTENER)) {
            agent->NotifyDisplayModeChanged(displayMode);
        }
    }
}

void ScreenSessionManagerAdapter::OnScreenshot(sptr<ScreenshotInfo> info)
{
    INIT_PROXY_CHECK_RETURN();
    if (info == nullptr) {
        TLOGE(WmsLogTag::DMS, "info is null");
        return;
    }
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREENSHOT_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    TLOGD(WmsLogTag::DMS, "start");
    for (auto& agent : agents) {
        agent->OnScreenshot(info);
    }
}

FoldStatus ScreenSessionManagerAdapter::FoldStatusTrans(FoldStatus foldStatus)
{
    FoldStatus transfoldstatus = foldStatus;
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        return transfoldstatus;
    }
    switch (foldStatus) {
        case FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_HALF_FOLDED:
        case FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_EXPAND:
            transfoldstatus = FoldStatus::EXPAND;
            break;
        default : {
            TLOGW(WmsLogTag::DMS, "foldStatus is unknown.");
        }
    }
    TLOGNFI(WmsLogTag::DMS,   "transfoldstatus:%{public}d ", transfoldstatus);
    return transfoldstatus;
}

void ScreenSessionManagerAdapter::NotifyFoldStatusChanged(FoldStatus foldStatus, FoldStatus lastStatus)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER);
    TLOGNFI(WmsLogTag::DMS,
        "foldStatus:%{public}d, agent size: %{public}u", foldStatus, static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    FoldStatus foldStatusNew = FoldStatus::UNKNOWN;
    bool isSysCall = false;
    for (auto& agent : agents) {
        foldStatusNew = foldStatus;
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        isSysCall = dmAgentContainer_.GetAgentSystem(agent);
        if (!isSysCall) {
            foldStatusNew = FoldStatusTrans(foldStatusNew);
        }
        if (ScreenSessionManager::GetInstance().IsFreezed(agentPid,
            DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER)) {
            continue;
        }
        bool isBlocked = ScreenSessionManager::GetInstance().IsHoverBlockPid(agentPid);
        if (!isBlocked) {
            agent->NotifyFoldStatusChanged(foldStatusNew);
            continue;
        }
        FoldStatus actualNewStatus =
            (isBlocked && foldStatusNew == FoldStatus::HALF_FOLD) ? FoldStatus::EXPAND : foldStatusNew;
        FoldStatus actualOldStatus =
            (isBlocked && lastStatus == FoldStatus::HALF_FOLD) ? FoldStatus::EXPAND : lastStatus;
        if (actualNewStatus == actualOldStatus) {
            continue;
        }
        agent->NotifyFoldStatusChanged(actualNewStatus);
    }
}

void ScreenSessionManagerAdapter::NotifyScreenMagneticStateChanged(bool isMagneticState)
{
    INIT_PROXY_CHECK_RETURN();
    TLOGNFI(WmsLogTag::DMS, "IsScreenMagneticState:%{public}u", static_cast<uint32_t>(isMagneticState));
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_MAGNETIC_STATE_CHANGED_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "NotifyScreenMagneticStateChanged agent is null");
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid,
            DisplayManagerAgentType::SCREEN_MAGNETIC_STATE_CHANGED_LISTENER)) {
            agent->NotifyScreenMagneticStateChanged(isMagneticState);
        }
    }
}

void ScreenSessionManagerAdapter::NotifyAvailableAreaChanged(DMRect area, DisplayId displayId)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::AVAILABLE_AREA_CHANGED_LISTENER);
    TLOGNFI(WmsLogTag::DMS, "entry, agent size: %{public}u", static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    TLOGNFI(WmsLogTag::DMS,
          "displayId: %{public}" PRIu64 ", AvailableArea: [%{public}d, %{public}d, %{public}u, %{public}u]",
          static_cast<DisplayId>(displayId), area.posX_, area.posY_, area.width_, area.height_);
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid,
                                                           DisplayManagerAgentType::AVAILABLE_AREA_CHANGED_LISTENER)) {
            agent->NotifyAvailableAreaChanged(area, displayId);
        }
    }
}

void ScreenSessionManagerAdapter::NotifyAbnormalScreenConnectChange(ScreenId screenId)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::ABNORMAL_SCREEN_CONNECT_CHANGE_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        agent->NotifyAbnormalScreenConnectChange(screenId);
    }
}
void ScreenSessionManagerAdapter::OnDisplayChangeForConcurrentUser(sptr<DisplayInfo> displayInfo,
    DisplayChangeEvent event)
{
    INIT_PROXY_CHECK_RETURN();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "Invalid display info");
        return;
    }

    DisplayId displayId = displayInfo->GetDisplayId();
    int32_t foregroundUserId =
        ScreenSessionManager::GetInstance().GetForegroundConcurrentUserWithVirtualScreen(displayId);
    TLOGNFI(
        WmsLogTag::DMS, "displayId: %{public}" PRIu64 ", foregroundUserId: %{public}d", displayId, foregroundUserId);

    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "event:%{public}d, displayId:%{public}" PRIu64 " agent is null", event, displayId);
        return;
    }

    std::ostringstream notifiedPids;
    for (auto& agent : agents) {
        if (agent == nullptr) {
            continue;
        }
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        AgentDisplayInfo agentDispInfo = GetAgentDisplayInfo(agent);
        int32_t agentUserId = agentDispInfo.userId;
        if (foregroundUserId != INVALID_USER_ID && agentUserId != INVALID_USER_ID &&
            foregroundUserId != USER_ID_DEFAULT && agentUserId != foregroundUserId) {
            TLOGD(WmsLogTag::DMS,
                "skip agent, agentUserId: %{public}d != foregroundUserId: %{public}d, displayId: %{public}" PRIu64,
                agentUserId, foregroundUserId, displayId);
            continue;
        }
        if (agentDispInfo.displayIds.find(displayId) == agentDispInfo.displayIds.end()) {
            TLOGD(
                WmsLogTag::DMS, "skip agent, displayId: %{public}" PRIu64 " not in agent's displayIds set", displayId);
            continue;
        }
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER)) {
            agent->OnDisplayChange(displayInfo, event);
            notifiedPids << " [pid:" << agentPid << ", uid:" << agentUserId << "]";
        }
    }
    TLOGNI(WmsLogTag::DMS,
        "event:%{public}d, displayId:%{public}" PRIu64 ", foregroundUserId: %{public}d, notified: [%{public}s]",
        event, displayId, foregroundUserId, notifiedPids.str().c_str());
}

void ScreenSessionManagerAdapter::SetAgentDisplayInfo(const sptr<IDisplayManagerAgent>& agent,
    int32_t userId, DisplayId displayId)
{
    if (agent == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(agentDisplayInfoMapMutex_);
    auto it = agentDisplayInfoMap_.find(agent);
    if (it != agentDisplayInfoMap_.end()) {
        it->second.userId = userId;
        it->second.displayIds.insert(displayId);
    } else {
        AgentDisplayInfo info;
        info.userId = userId;
        info.displayIds.insert(displayId);
        agentDisplayInfoMap_[agent] = info;
    }
    TLOGD(WmsLogTag::DMS, "set agent userId: %{public}d, displayId: %{public}" PRIu64, userId, displayId);
}

AgentDisplayInfo ScreenSessionManagerAdapter::GetAgentDisplayInfo(const sptr<IDisplayManagerAgent>& agent)
{
    std::lock_guard<std::mutex> lock(agentDisplayInfoMapMutex_);
    auto it = agentDisplayInfoMap_.find(agent);
    if (it != agentDisplayInfoMap_.end()) {
        return it->second;
    }
    return AgentDisplayInfo{ INVALID_USER_ID, {} };
}

void ScreenSessionManagerAdapter::RemoveAgentDisplayInfo(const sptr<IDisplayManagerAgent>& agent)
{
    if (agent == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(agentDisplayInfoMapMutex_);
    agentDisplayInfoMap_.erase(agent);
    TLOGD(WmsLogTag::DMS, "remove agent display info mapping");
}

std::set<sptr<IDisplayManagerAgent>> ScreenSessionManagerAdapter::GetFilteredAgents(DisplayManagerAgentType type,
    DisplayId displayId, int32_t foregroundUserId)
{
    std::set<sptr<IDisplayManagerAgent>> filteredAgents;
    auto agents = dmAgentContainer_.GetAgentsByType(type);
    for (auto& agent : agents) {
        if (agent == nullptr) {
            continue;
        }
        AgentDisplayInfo agentDispInfo = GetAgentDisplayInfo(agent);
        int32_t agentUserId = agentDispInfo.userId;

        if (foregroundUserId != INVALID_USER_ID && agentUserId != INVALID_USER_ID && agentUserId != foregroundUserId) {
            TLOGD(WmsLogTag::DMS, "skip agent, agentUserId: %{public}d != foregroundUserId: %{public}d",
                agentUserId, foregroundUserId);
            continue;
        }

        if (agentDispInfo.displayIds.find(displayId) == agentDispInfo.displayIds.end()) {
            TLOGD(
                WmsLogTag::DMS, "skip agent, displayId: %{public}" PRIu64 " not in agent's displayIds set", displayId);
            continue;
        }

        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid, type)) {
            filteredAgents.insert(agent);
        }
    }
    TLOGD(WmsLogTag::DMS, "GetFilteredAgents type: %{public}u, displayId: %{public}" PRIu64
        ", foregroundUserId: %{public}d, filtered count: %{public}zu",
        type, displayId, foregroundUserId, filteredAgents.size());
    return filteredAgents;
}

void ScreenSessionManagerAdapter::OnDisplayAttributeChangeForConcurrentUser(sptr<DisplayInfo> displayInfo,
    const std::vector<std::string>& attributes, DisplayId displayId, int32_t uid)
{
    if (uid != INVALID_UID) {
        TLOGD(WmsLogTag::DMS, "Notify display attribute change with uid: %{public}d", uid);
    }
    INIT_PROXY_CHECK_RETURN();
    if (displayInfo == nullptr || attributes.empty()) {
        TLOGE(WmsLogTag::DMS, "displayInfo or attributes is empty");
        return;
    }

    int32_t foregroundUserId =
        ScreenSessionManager::GetInstance().GetForegroundConcurrentUserWithVirtualScreen(displayId);
    TLOGNFI(
        WmsLogTag::DMS, "displayId: %{public}" PRIu64 ", foregroundUserId: %{public}d", displayId, foregroundUserId);

    std::set<sptr<IDisplayManagerAgent>> agents;
    dmAttributeAgentContainer_.ParseAttributeAgentsMap([&](auto& map) {
        for (auto it = map.begin(); it != map.end(); ++it) {
            if (IsAgentListenedAttributes(it->second.second, attributes)) {
                agents.insert(it->second.first);
            }
        }
    });

    for (auto& agent : agents) {
        if (agent == nullptr) {
            continue;
        }
        AgentDisplayInfo agentDispInfo = GetAgentDisplayInfo(agent);
        int32_t agentUserId = agentDispInfo.userId;

        if (foregroundUserId != INVALID_USER_ID && agentUserId != INVALID_USER_ID &&
            foregroundUserId != USER_ID_DEFAULT && agentUserId != foregroundUserId) {
            TLOGD(WmsLogTag::DMS, "skip agent, agentUserId: %{public}d != foregroundUserId: %{public}d",
                agentUserId, foregroundUserId);
            continue;
        }
        if (agentDispInfo.displayIds.find(displayId) == agentDispInfo.displayIds.end()) {
            TLOGD(
                WmsLogTag::DMS, "skip agent, displayId: %{public}" PRIu64 " not in agent's displayIds set", displayId);
            continue;
        }
        int32_t agentPid = dmAttributeAgentContainer_.GetAgentPid(agent);
        if (uid != INVALID_UID && ScreenSessionManager::GetInstance().GetStoredPidFromUid(uid, agentPid)) {
            TLOGI(WmsLogTag::DMS, "Is not current uid, do not need to notify");
            continue;
        }
        if (!ScreenSessionManager::GetInstance().IsFreezed(
            agentPid, DisplayManagerAgentType::DISPLAY_ATTRIBUTE_CHANGED_LISTENER)) {
            agent->OnDisplayAttributeChange(displayInfo, attributes);
        }
    }
}

void ScreenSessionManagerAdapter::OnScreenChangeForConcurrentUser(sptr<ScreenInfo> screenInfo,
    ScreenChangeEvent event, DisplayId displayId)
{
    INIT_PROXY_CHECK_RETURN();
    if (screenInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "error, screenInfo is nullptr.");
        return;
    }

    int32_t foregroundUserId =
        ScreenSessionManager::GetInstance().GetForegroundConcurrentUserWithVirtualScreen(displayId);
    TLOGNFI(WmsLogTag::DMS, "screenId:%{public}" PRIu64 ", displayId:%{public}" PRIu64 ", UserId:%{public}d",
        screenInfo->GetScreenId(), displayId, foregroundUserId);

    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "screenId:%{public}" PRIu64 ", agent is null", screenInfo->GetScreenId());
        return;
    }

    std::ostringstream notifiedPids;
    for (auto& agent : agents) {
        if (agent == nullptr) {
            continue;
        }
        AgentDisplayInfo agentDispInfo = GetAgentDisplayInfo(agent);
        int32_t agentUserId = agentDispInfo.userId;

        if (foregroundUserId != INVALID_USER_ID && agentUserId != INVALID_USER_ID &&
            foregroundUserId != USER_ID_DEFAULT && agentUserId != foregroundUserId) {
            TLOGD(WmsLogTag::DMS,
                  "skip agent, agentUserId:%{public}d != foregroundUserId:%{public}d",
                  agentUserId,
                  foregroundUserId);
            continue;
        }
        if (agentDispInfo.displayIds.find(displayId) == agentDispInfo.displayIds.end()) {
            TLOGD(WmsLogTag::DMS, "skip agent, displayId:%{public}" PRIu64 " not in agent's displayIds set", displayId);
            continue;
        }
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid, DisplayManagerAgentType::SCREEN_EVENT_LISTENER)) {
            agent->OnScreenChange(screenInfo, event);
            notifiedPids << " [pid:" << agentPid << ", uid:" << agentUserId << "]";
        }
    }
    TLOGNI(WmsLogTag::DMS, "screenId:%{public}" PRIu64 ", agent size:%{public}u, notified:[%{public}s]",
        screenInfo->GetScreenId(), static_cast<uint32_t>(agents.size()), notifiedPids.str().c_str());
}

void ScreenSessionManagerAdapter::OnDisplayCreateForConcurrentUser(sptr<DisplayInfo> displayInfo)
{
    INIT_PROXY_CHECK_RETURN();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayInfo nullptr");
        return;
    }

    DisplayId displayId = displayInfo->GetDisplayId();
    int32_t foregroundUserId =
        ScreenSessionManager::GetInstance().GetForegroundConcurrentUserWithVirtualScreen(displayId);
    TLOGNFI(WmsLogTag::DMS, "displayId: %{public}" PRIu64 ", UserId: %{public}d", displayId, foregroundUserId);
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    TLOGNFI(WmsLogTag::DMS, "start, agent size: %{public}u", static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        AgentDisplayInfo agentDispInfo = GetAgentDisplayInfo(agent);
        int32_t agentUserId = agentDispInfo.userId;
        if (foregroundUserId != INVALID_USER_ID && agentUserId != INVALID_USER_ID &&
            foregroundUserId != USER_ID_DEFAULT && agentUserId != foregroundUserId) {
            TLOGD(WmsLogTag::DMS, "skip agent, agentUserId:%{public}d != UserId:%{public}d",
                agentUserId, foregroundUserId);
            continue;
        }
        if (agentDispInfo.displayIds.find(displayId) == agentDispInfo.displayIds.end()) {
            continue;
        }
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER)) {
            agent->OnDisplayCreate(displayInfo);
        }
    }
}

void ScreenSessionManagerAdapter::OnDisplayDestroyForConcurrentUser(DisplayId displayId)
{
    INIT_PROXY_CHECK_RETURN();
    int32_t foregroundUserId =
        ScreenSessionManager::GetInstance().GetForegroundConcurrentUserWithVirtualScreen(displayId);
    TLOGNFI(WmsLogTag::DMS, "displayId: %{public}" PRIu64 ", UserId: %{public}d", displayId, foregroundUserId);
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    TLOGNFI(WmsLogTag::DMS, "agent size: %{public}u", static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        AgentDisplayInfo agentDispInfo = GetAgentDisplayInfo(agent);
        int32_t agentUserId = agentDispInfo.userId;

        if (foregroundUserId != INVALID_USER_ID && agentUserId != INVALID_USER_ID &&
            foregroundUserId != USER_ID_DEFAULT && agentUserId != foregroundUserId) {
            TLOGD(WmsLogTag::DMS, "skip agent, agentUserId:%{public}d != UserId:%{public}d",
                agentUserId, foregroundUserId);
            continue;
        }

        if (agentDispInfo.displayIds.find(displayId) == agentDispInfo.displayIds.end()) {
            continue;
        }
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER)) {
            agent->OnDisplayDestroy(displayId);
        }
    }
}

void ScreenSessionManagerAdapter::OnScreenConnectForConcurrentUser(const sptr<ScreenInfo> screenInfo,
    DisplayId displayId)
{
    INIT_PROXY_CHECK_RETURN();
    if (screenInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenInfo nullptr");
        return;
    }

    int32_t foregroundUserId =
        ScreenSessionManager::GetInstance().GetForegroundConcurrentUserWithVirtualScreen(displayId);
    TLOGNFI(WmsLogTag::DMS, "screenId:%{public}" PRIu64 ", displayId:%{public}" PRIu64 ", UserId:%{public}d",
        screenInfo->GetScreenId(), displayId, foregroundUserId);

    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }

    for (auto& agent : agents) {
        if (agent == nullptr) {
            continue;
        }
        AgentDisplayInfo agentDispInfo = GetAgentDisplayInfo(agent);
        int32_t agentUserId = agentDispInfo.userId;
        if (foregroundUserId != INVALID_USER_ID && agentUserId != INVALID_USER_ID &&
            foregroundUserId != USER_ID_DEFAULT && agentUserId != foregroundUserId) {
            TLOGD(WmsLogTag::DMS, "skip agent, agentUserId:%{public}d != foregroundUserId:%{public}d",
                agentUserId, foregroundUserId);
            continue;
        }
        if (agentDispInfo.displayIds.find(displayId) == agentDispInfo.displayIds.end()) {
            continue;
        }
        agent->OnScreenConnect(screenInfo);
    }
}

void ScreenSessionManagerAdapter::OnScreenDisconnectForConcurrentUser(ScreenId screenId, DisplayId displayId)
{
    INIT_PROXY_CHECK_RETURN();

    int32_t foregroundUserId =
        ScreenSessionManager::GetInstance().GetForegroundConcurrentUserWithVirtualScreen(displayId);
    TLOGNFI(WmsLogTag::DMS, "screenId:%{public}" PRIu64 ", displayId:%{public}" PRIu64 ", UserId:%{public}d",
        screenId, displayId, foregroundUserId);

    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }

    for (auto& agent : agents) {
        if (agent == nullptr) {
            continue;
        }
        AgentDisplayInfo agentDispInfo = GetAgentDisplayInfo(agent);
        int32_t agentUserId = agentDispInfo.userId;
        if (foregroundUserId != INVALID_USER_ID && agentUserId != INVALID_USER_ID &&
            foregroundUserId != USER_ID_DEFAULT && agentUserId != foregroundUserId) {
            TLOGD(WmsLogTag::DMS, "skip agent, agentUserId:%{public}d != foregroundUserId:%{public}d",
                agentUserId, foregroundUserId);
            continue;
        }
        if (agentDispInfo.displayIds.find(displayId) == agentDispInfo.displayIds.end()) {
            continue;
        }
        agent->OnScreenDisconnect(screenId);
    }
}
} // namespace Rosen
} // namespace OHOS