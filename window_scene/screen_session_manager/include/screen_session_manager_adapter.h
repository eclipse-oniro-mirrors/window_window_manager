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
#include "dm_common.h"
#include "wm_single_instance.h"
#include "client_agent_container.h"
#include "zidl/idisplay_manager_agent.h"

namespace OHOS {
namespace Rosen {
class ScreenSessionManagerAdapter {
WM_DECLARE_SINGLE_INSTANCE_BASE(ScreenSessionManagerAdapter)

public:
    DMError RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type);
    DMError UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type);

    void NotifyFoldStatusChanged(FoldStatus foldStatus);
    void OnScreenGroupChange(const std::string&, const sptr<ScreenInfo>&, ScreenGroupChangeEvent);
    void OnScreenGroupChange(const std::string&, const std::vector<sptr<ScreenInfo>>&, ScreenGroupChangeEvent);
    void OnScreenDisconnect(ScreenId);
    void OnDisplayDestroy(DisplayId);
    void OnDisplayCreate(sptr<DisplayInfo>);
    bool NotifyDisplayStateChanged(DisplayId id, DisplayState state);
    void OnScreenChange(sptr<ScreenInfo>, ScreenChangeEvent);
    void OnDisplayChange(sptr<DisplayInfo>, DisplayChangeEvent event, int32_t uid);
    void OnDisplayChange(sptr<DisplayInfo>, DisplayChangeEvent event);
    void NotifyScreenModeChange(const std::vector<sptr<ScreenInfo>> screenInfos);
    void NotifyScreenChanged(sptr<ScreenInfo> screenInfo, ScreenChangeEvent event);
    bool NotifyDisplayPowerEvent(DisplayPowerEvent event, EventStatus status);
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
    ClientAgentContainer<IDisplayManagerAgent, DisplayManagerAgentType> dmAgentContainer_;
    ClientAgentContainer<IDisplayManagerAgent, std::string> dmAttributeAgentContainer_;
    void NotifyAbnormalScreenConnectChange(ScreenId screenId);
    void OnDisplayAttributeChange(sptr<DisplayInfo> displayInfo, const std::vector<std::string>& attributes);
    bool IsAgentListenedAttributes(std::set<std::string>& listenedAttributes,
        const std::vector<std::string>& attributes);
private:
    ScreenSessionManagerAdapter() {}
    virtual ~ScreenSessionManagerAdapter() = default;
};
}
}
#endif // SCREEN_SESSION_MANAGER_AGENT_H

