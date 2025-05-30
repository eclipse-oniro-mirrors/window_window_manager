/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_DISPLAY_MANAGER_AGENT_CONTROLLER_H
#define OHOS_ROSEN_DISPLAY_MANAGER_AGENT_CONTROLLER_H

#include <mutex>
#include "dm_common.h"
#include "wm_single_instance.h"
#include "client_agent_container.h"
#include "zidl/idisplay_manager_agent.h"

namespace OHOS {
namespace Rosen {
class DisplayManagerAgentController {
WM_DECLARE_SINGLE_INSTANCE_BASE(DisplayManagerAgentController)
public:
    DMError RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type);
    DMError UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type);

    bool NotifyDisplayPowerEvent(DisplayPowerEvent event, EventStatus status);
    bool NotifyDisplayStateChanged(DisplayId id, DisplayState state);

    void OnScreenConnect(sptr<ScreenInfo> screenInfo);
    void OnScreenDisconnect(ScreenId);
    void OnScreenChange(sptr<ScreenInfo>, ScreenChangeEvent);
    void OnScreenGroupChange(const std::string&, const sptr<ScreenInfo>&, ScreenGroupChangeEvent);
    void OnScreenGroupChange(const std::string&, const std::vector<sptr<ScreenInfo>>&, ScreenGroupChangeEvent);
    void OnDisplayCreate(sptr<DisplayInfo>);
    void OnDisplayDestroy(DisplayId);
    void OnDisplayChange(sptr<DisplayInfo>, DisplayChangeEvent);
    void OnScreenshot(sptr<ScreenshotInfo>);
    void NotifyPrivateWindowStateChanged(bool hasPrivate);
    void NotifyFoldStatusChanged(FoldStatus);
    void NotifyFoldAngleChanged(std::vector<float> foldAngles);
    void NotifyCaptureStatusChanged(bool isCapture);
    void NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info);
    void NotifyDisplayModeChanged(FoldDisplayMode);
    void NotifyAvailableRectChanged(DMRect);

private:
    DisplayManagerAgentController() {}
    virtual ~DisplayManagerAgentController() = default;

    ClientAgentContainer<IDisplayManagerAgent, DisplayManagerAgentType> dmAgentContainer_;
};
}
}
#endif // OHOS_ROSEN_DISPLAY_MANAGER_AGENT_CONTROLLER_H
