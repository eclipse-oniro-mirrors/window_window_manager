/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_DISPLAY_MANAGER_AGRNT_PROXY_H
#define OHOS_ROSEN_DISPLAY_MANAGER_AGRNT_PROXY_H

#include <iremote_proxy.h>
#include "dm_common.h"
#include "idisplay_manager_agent.h"

namespace OHOS {
namespace Rosen {
class DisplayManagerAgentProxy : public IRemoteProxy<IDisplayManagerAgent> {
public:
    explicit DisplayManagerAgentProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IDisplayManagerAgent>(impl) {};
    ~DisplayManagerAgentProxy() = default;

    void NotifyDisplayPowerEvent(DisplayPowerEvent event, EventStatus status) override;
    void NotifyDisplayStateChanged(DisplayId id, DisplayState state) override;
    void OnScreenConnect(sptr<ScreenInfo>) override;
    void OnScreenDisconnect(ScreenId) override;
    void OnScreenChange(const sptr<ScreenInfo>&, ScreenChangeEvent) override;
    void OnScreenGroupChange(const std::string& trigger,
        const std::vector<sptr<ScreenInfo>>&, ScreenGroupChangeEvent) override;
    void OnDisplayCreate(sptr<DisplayInfo>) override;
    void OnDisplayDestroy(DisplayId) override;
    void OnDisplayChange(sptr<DisplayInfo>, DisplayChangeEvent) override;
    void OnScreenshot(sptr<ScreenshotInfo>) override;
    void NotifyPrivateWindowStateChanged(bool hasPrivate) override;
    void NotifyPrivateStateWindowListChanged(DisplayId id, std::vector<std::string> privacyWindowList) override;
    void NotifyFoldStatusChanged(FoldStatus) override;
    void NotifyFoldAngleChanged(std::vector<float> foldAngles) override;
    void NotifyCaptureStatusChanged(bool isCapture) override;
    void NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info) override;
    void NotifyDisplayModeChanged(FoldDisplayMode) override;
    void NotifyAvailableAreaChanged(DMRect, DisplayId displayId) override;
    void NotifyScreenMagneticStateChanged(bool isMagneticState) override;
    void NotifyScreenModeChange(const std::vector<sptr<ScreenInfo>>& screenInfos) override;
    void NotifyAbnormalScreenConnectChange(ScreenId screenId) override;
    void NotifyBrightnessInfoChanged(ScreenId screenId, const ScreenBrightnessInfo& info) override;
    virtual void NotifyRecordingDisplayChanged(const std::vector<DisplayId>& displayIds) override;
    void OnDisplayAttributeChange(sptr<DisplayInfo> displayInfo, const std::vector<std::string>& attributes)
        override;
private:
    static inline BrokerDelegator<DisplayManagerAgentProxy> delegator_;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_DISPLAY_MANAGER_AGRNT_PROXY_H
