/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "display_manager_agent_controller.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(DisplayManagerAgentController)

DMError DisplayManagerAgentController::RegisterDisplayManagerAgent(
    const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
{
    return dmAgentContainer_.RegisterAgent(displayManagerAgent, type) ? DMError::DM_OK :DMError::DM_ERROR_NULLPTR;
}

DMError DisplayManagerAgentController::UnregisterDisplayManagerAgent(
    const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
{
    return dmAgentContainer_.UnregisterAgent(displayManagerAgent, type) ? DMError::DM_OK :DMError::DM_ERROR_NULLPTR;
}

bool DisplayManagerAgentController::NotifyDisplayPowerEvent(DisplayPowerEvent event, EventStatus status)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER);
    if (agents.empty()) {
        return false;
    }
    TLOGI(WmsLogTag::DMS, "enter");
    for (auto& agent : agents) {
        agent->NotifyDisplayPowerEvent(event, status);
    }
    return true;
}

bool DisplayManagerAgentController::NotifyDisplayStateChanged(DisplayId id, DisplayState state)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_STATE_LISTENER);
    if (agents.empty()) {
        return false;
    }
    TLOGI(WmsLogTag::DMS, "enter");
    for (auto& agent : agents) {
        agent->NotifyDisplayStateChanged(id, state);
    }
    return true;
}

void DisplayManagerAgentController::OnScreenConnect(sptr<ScreenInfo> screenInfo)
{
    if (screenInfo == nullptr) {
        return;
    }
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    if (agents.empty()) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "enter");
    for (auto& agent : agents) {
        agent->OnScreenConnect(screenInfo);
    }
}

void DisplayManagerAgentController::OnScreenDisconnect(ScreenId screenId)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    if (agents.empty()) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "enter");
    for (auto& agent : agents) {
        agent->OnScreenDisconnect(screenId);
    }
}

void DisplayManagerAgentController::OnScreenChange(sptr<ScreenInfo> screenInfo, ScreenChangeEvent screenChangeEvent)
{
    if (screenInfo == nullptr) {
        return;
    }
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    if (agents.empty()) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "enter");
    for (auto& agent : agents) {
        agent->OnScreenChange(screenInfo, screenChangeEvent);
    }
}

void DisplayManagerAgentController::OnScreenGroupChange(const std::string& trigger,
    const sptr<ScreenInfo>& screenInfo, ScreenGroupChangeEvent groupEvent)
{
    if (screenInfo == nullptr) {
        return;
    }
    std::vector<sptr<ScreenInfo>> screenInfos;
    screenInfos.push_back(screenInfo);
    OnScreenGroupChange(trigger, screenInfos, groupEvent);
}

void DisplayManagerAgentController::OnScreenGroupChange(const std::string& trigger,
    const std::vector<sptr<ScreenInfo>>& screenInfos, ScreenGroupChangeEvent groupEvent)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    std::vector<sptr<ScreenInfo>> infos;
    for (auto& screenInfo : screenInfos) {
        if (screenInfo != nullptr) {
            infos.emplace_back(screenInfo);
        }
    }
    if (agents.empty() || infos.empty()) {
        return;
    }
    for (auto& agent : agents) {
        agent->OnScreenGroupChange(trigger, infos, groupEvent);
    }
}

void DisplayManagerAgentController::OnDisplayCreate(sptr<DisplayInfo> displayInfo)
{
    if (displayInfo == nullptr) {
        return;
    }
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    if (agents.empty()) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "enter");
    for (auto& agent : agents) {
        agent->OnDisplayCreate(displayInfo);
    }
}

void DisplayManagerAgentController::OnDisplayDestroy(DisplayId displayId)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    if (agents.empty()) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "enter");
    for (auto& agent : agents) {
        agent->OnDisplayDestroy(displayId);
    }
}

void DisplayManagerAgentController::OnDisplayChange(
    sptr<DisplayInfo> displayInfo, DisplayChangeEvent displayChangeEvent)
{
    if (displayInfo == nullptr) {
        return;
    }
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    if (agents.empty()) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "enter");
    for (auto& agent : agents) {
        agent->OnDisplayChange(displayInfo, displayChangeEvent);
    }
}

void DisplayManagerAgentController::OnScreenshot(sptr<ScreenshotInfo> info)
{
    if (info == nullptr) {
        return;
    }
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREENSHOT_EVENT_LISTENER);
    if (agents.empty()) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "enter");
    for (auto& agent : agents) {
        agent->OnScreenshot(info);
    }
}

void DisplayManagerAgentController::NotifyPrivateWindowStateChanged(bool hasPrivate)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::PRIVATE_WINDOW_LISTENER);
    if (agents.empty()) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "PrivateWindow status : %{public}u", hasPrivate);
    for (auto& agent : agents) {
        agent->NotifyPrivateWindowStateChanged(hasPrivate);
    }
}

void DisplayManagerAgentController::NotifyFoldStatusChanged(FoldStatus foldStatus)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER);
    if (agents.empty()) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "FoldStatus is : %{public}u", foldStatus);
    for (auto& agent : agents) {
        agent->NotifyFoldStatusChanged(foldStatus);
    }
}

void DisplayManagerAgentController::NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_UPDATE_LISTENER);
    if (agents.empty()) {
        return;
    }
    for (auto& agent : agents) {
        agent->NotifyDisplayChangeInfoChanged(info);
    }
}

void DisplayManagerAgentController::NotifyDisplayModeChanged(FoldDisplayMode displayMode)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_MODE_CHANGED_LISTENER);
    if (agents.empty()) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "FoldDisplayMode is : %{public}u", displayMode);
    for (auto& agent : agents) {
        agent->NotifyDisplayModeChanged(displayMode);
    }
}
}
}