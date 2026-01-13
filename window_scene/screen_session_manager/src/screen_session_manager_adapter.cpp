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
        TLOGE(WmsLogTag::DMS, "agent is null");
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
    if (event == DisplayChangeEvent::UPDATE_REFRESHRATE) {
        TLOGND(WmsLogTag::DMS,
               "event:%{public}d, displayId:%{public}" PRIu64 ", agent size: %{public}u",
               event, displayInfo->GetDisplayId(), static_cast<uint32_t>(agents.size()));
    } else {
        TLOGNI(WmsLogTag::DMS,
               "event:%{public}d, displayId:%{public}" PRIu64 ", agent size: %{public}u",
               event, displayInfo->GetDisplayId(), static_cast<uint32_t>(agents.size()));
    }
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "OnDisplayChange agent is null");
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER)) {
            agent->OnDisplayChange(displayInfo, event);
        }
    }
}

void ScreenSessionManagerAdapter::OnDisplayAttributeChange(sptr<DisplayInfo> displayInfo,
    const std::vector<std::string>& attributes)
{
    INIT_PROXY_CHECK_RETURN();
    if (attributes.empty()) {
        TLOGE(WmsLogTag::DMS, "attributes is empty");
        return;
    }
 
    std::set<sptr<IDisplayManagerAgent>> agents;
    auto agentsMap = dmAttributeAgentContainer_.GetAttributeAgentsMap();
    for (auto it = agentsMap.begin(); it != agentsMap.end(); ++it) {
        if (IsAgentListenedAttributes(it->second.second, attributes)) {
            agents.insert(it->second.first);
        }
    }
 
    for (auto& agent : agents) {
        int32_t agentPid = dmAttributeAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid,
            DisplayManagerAgentType::DISPLAY_ATTRIBUTE_CHANGED_LISTENER)) {
            agent->OnDisplayAttributeChange(displayInfo, attributes);
        }
    }
}

bool ScreenSessionManagerAdapter::IsAgentListenedAttributes(std::set<std::string>& listendAttributes,
    const std::vector<std::string>& attributes)
{
    for (auto attribute : attributes) {
        if (listendAttributes.find(attribute) != listendAttributes.end()) {
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
    TLOGNI(WmsLogTag::DMS, "screenId:%{public}" PRIu64 ", agent size: %{public}u",
        screenInfo->GetScreenId(), static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid, DisplayManagerAgentType::SCREEN_EVENT_LISTENER)) {
            agent->OnScreenChange(screenInfo, event);
        }
    }
}

bool ScreenSessionManagerAdapter::NotifyDisplayPowerEvent(DisplayPowerEvent event, EventStatus status)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return false;
    }
    TLOGI(WmsLogTag::DMS, "Received Display Power Event: %{public}d", static_cast<int>(event));
    for (auto& agent : agents) {
        agent->NotifyDisplayPowerEvent(event, status);
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
    TLOGI(WmsLogTag::DMS,
        "Display State Changed: ID=%{public}d State=%{public}d",
        static_cast<uint32_t>(id), static_cast<uint32_t>(state));
    for (auto& agent : agents) {
        agent->NotifyDisplayStateChanged(id, state);
    }
    return true;
}

void ScreenSessionManagerAdapter::OnDisplayCreate(sptr<DisplayInfo> displayInfo)
{
    INIT_PROXY_CHECK_RETURN();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenInfo nullptr");
        return;
    }
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    TLOGI(WmsLogTag::DMS, "start, agent size: %{public}u", static_cast<uint32_t>(agents.size()));
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
    TLOGI(WmsLogTag::DMS, "agent size: %{public}u", static_cast<uint32_t>(agents.size()));
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
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }

    TLOGI(WmsLogTag::DMS, "Private Window Status: %s", hasPrivate ? "Active" : "Inactive");
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
        TLOGE(WmsLogTag::DMS, "agent is null");
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
        TLOGE(WmsLogTag::DMS, "agent or infos is null");
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
    TLOGI(WmsLogTag::DMS, "start");
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
    TLOGI(WmsLogTag::DMS, "start");
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
    TLOGI(WmsLogTag::DMS,
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
    TLOGI(WmsLogTag::DMS, "start");
    for (auto& agent : agents) {
        agent->OnScreenshot(info);
    }
}

void ScreenSessionManagerAdapter::NotifyFoldStatusChanged(FoldStatus foldStatus)
{
    INIT_PROXY_CHECK_RETURN();
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER);
    TLOGI(WmsLogTag::DMS,
          "foldStatus:%{public}d, agent size: %{public}u", foldStatus, static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!ScreenSessionManager::GetInstance().IsFreezed(agentPid,
                                                           DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER)) {
            agent->NotifyFoldStatusChanged(foldStatus);
        }
    }
}

void ScreenSessionManagerAdapter::NotifyScreenMagneticStateChanged(bool isMagneticState)
{
    INIT_PROXY_CHECK_RETURN();
    TLOGI(WmsLogTag::DMS, "IsScreenMagneticState:%{public}u", static_cast<uint32_t>(isMagneticState));
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
    TLOGI(WmsLogTag::DMS, "entry, agent size: %{public}u", static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agent is null");
        return;
    }
    TLOGI(WmsLogTag::DMS,
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

} // namespace Rosen
} // namespace OHOS