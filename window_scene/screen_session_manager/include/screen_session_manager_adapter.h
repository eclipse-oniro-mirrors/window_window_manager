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

#ifndef SCREEN_SESSION_MANAGER_ADAPTER_H
#define SCREEN_SESSION_MANAGER_ADAPTER_H

#include <mutex>
#include <map>
#include <set>
#include "dm_common.h"
#include "wm_common.h"
#include "wm_single_instance.h"
#include "client_agent_container.h"
#include "zidl/idisplay_manager_agent.h"

namespace OHOS {
namespace Rosen {

struct AgentDisplayInfo {
    int32_t userId;
    std::set<DisplayId> displayIds;
};

class ScreenSessionManagerAdapter {
WM_DECLARE_SINGLE_INSTANCE_BASE(ScreenSessionManagerAdapter)

public:
    DMError RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type);
    DMError UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type);

    void NotifyFoldStatusChanged(FoldStatus foldStatus, FoldStatus lastStatus = FoldStatus::UNKNOWN);
    FoldStatus FoldStatusTrans(FoldStatus foldStatus);
    void OnScreenGroupChange(const std::string&, const sptr<ScreenInfo>&, ScreenGroupChangeEvent);
    void OnScreenGroupChange(const std::string&, const std::vector<sptr<ScreenInfo>>&, ScreenGroupChangeEvent);
    void OnScreenDisconnect(ScreenId);
    void OnDisplayDestroy(DisplayId);
    void OnDisplayCreate(sptr<DisplayInfo>);
    bool NotifyDisplayStateChanged(DisplayId id, DisplayState state);
    bool NotifyDisplayStateChangedById(DisplayId displayId, DisplayState state);
    void OnScreenChange(sptr<ScreenInfo>, ScreenChangeEvent);
    void OnDisplayChange(sptr<DisplayInfo>, DisplayChangeEvent event, int32_t uid);
    void OnDisplayChange(sptr<DisplayInfo>, DisplayChangeEvent event);
    void NotifyScreenModeChange(const std::vector<sptr<ScreenInfo>> screenInfos);
    void NotifyScreenChanged(sptr<ScreenInfo> screenInfo, ScreenChangeEvent event);
    bool NotifyDisplayPowerEvent(DisplayPowerEvent event, EventStatus status);
    bool NotifySpecifiedDisplayPowerEvent(DisplayId displayId, DisplayPowerEvent event, EventStatus status);
    void NotifyPrivateWindowStateChanged(bool hasPrivate);
    void NotifyPrivateStateWindowListChanged(DisplayId id, std::vector<std::string> privacyWindowList);
    void OnScreenConnect(const sptr<ScreenInfo> screenInfo);
    void OnScreenshot(sptr<ScreenshotInfo>);
    void NotifyCaptureStatusChanged(bool isCapture);
    void NotifyCaptureStatusChanged();
    void NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info);
    void NotifyFoldAngleChanged(std::vector<float> foldAngles);
    void NotifyDisplayModeChanged(FoldDisplayMode displayMode);
    void NotifyScreenMagneticStateChanged(bool isMagneticState);
    void NotifyAvailableAreaChanged(DMRect area, DisplayId displayId);
    void NotifyAvailableAreaChanged(DMRect area, DisplayId displayId, int32_t uid);
    ClientAgentContainer<IDisplayManagerAgent, DisplayManagerAgentType> dmAgentContainer_;
    ClientAgentContainer<IDisplayManagerAgent, std::string> dmAttributeAgentContainer_;
    void NotifyAbnormalScreenConnectChange(ScreenId screenId);
    void OnDisplayAttributeChange(sptr<DisplayInfo> displayInfo, const std::vector<std::string>& attributes,
        int32_t uid = INVALID_UID);
    bool IsAgentListenedAttributes(std::set<std::string>& listenedAttributes,
        const std::vector<std::string>& attributes);
    void OnScreenDisconnectForConcurrentUser(ScreenId screenId, DisplayId displayId);
    void OnDisplayDestroyForConcurrentUser(DisplayId);
    void OnDisplayCreateForConcurrentUser(sptr<DisplayInfo>);
    void OnScreenChangeForConcurrentUser(sptr<ScreenInfo> screenInfo, ScreenChangeEvent event, DisplayId displayId);
    void OnDisplayChangeForConcurrentUser(sptr<DisplayInfo> displayInfo, DisplayChangeEvent event);
    void OnScreenConnectForConcurrentUser(const sptr<ScreenInfo> screenInfo, DisplayId displayId);
    void OnDisplayAttributeChangeForConcurrentUser(sptr<DisplayInfo> displayInfo,
        const std::vector<std::string>& attributes, DisplayId displayId, int32_t uid = INVALID_UID);
    void SetAgentDisplayInfo(const sptr<IDisplayManagerAgent>& agent, int32_t userId, DisplayId displayId);
    AgentDisplayInfo GetAgentDisplayInfo(const sptr<IDisplayManagerAgent>& agent);
    void RemoveAgentDisplayInfo(const sptr<IDisplayManagerAgent>& agent);
    std::set<sptr<IDisplayManagerAgent>> GetFilteredAgents(
        DisplayManagerAgentType type, DisplayId displayId, int32_t foregroundUserId);
private:
    ScreenSessionManagerAdapter() {}
    virtual ~ScreenSessionManagerAdapter() = default;
    std::mutex agentDisplayInfoMapMutex_;
    std::map<sptr<IDisplayManagerAgent>, AgentDisplayInfo> agentDisplayInfoMap_;
};
}
}
#endif // SCREEN_SESSION_MANAGER_AGENT_H

