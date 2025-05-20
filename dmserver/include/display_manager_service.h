/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_DMSERVER_DISPLAY_MANAGER_SERVICE_H
#define FOUNDATION_DMSERVER_DISPLAY_MANAGER_SERVICE_H

#include <map>
#include <mutex>

#include <system_ability.h>
#include <surface.h>
#include <ui/rs_display_node.h>

#include "atomic_map.h"
#include "dm_common.h"
#include "display_dumper.h"
#include "screen.h"
#include "abstract_display.h"
#include "abstract_display_controller.h"
#include "abstract_screen_controller.h"
#include "display_change_listener.h"
#include "display_cutout_controller.h"
#include "display_manager_stub.h"
#include "display_power_controller.h"
#include "singleton_delegator.h"
#include "window_info_queried_listener.h"

namespace OHOS::Rosen {
class DisplayManagerService : public SystemAbility, public DisplayManagerStub {
DECLARE_SYSTEM_ABILITY(DisplayManagerService);
WM_DECLARE_SINGLE_INSTANCE_BASE(DisplayManagerService);

public:
    int Dump(int fd, const std::vector<std::u16string>& args) override;
    void OnStart() override;
    void OnStop() override;
    ScreenId CreateVirtualScreen(VirtualScreenOption option,
        const sptr<IRemoteObject>& displayManagerAgent) override;
    DMError DestroyVirtualScreen(ScreenId screenId) override;
    DMError SetVirtualScreenSurface(ScreenId screenId, sptr<IBufferProducer> surface) override;
    DMError IsScreenRotationLocked(bool& isLocked) override;
    DMError SetScreenRotationLocked(bool isLocked) override;
    DMError SetScreenRotationLockedFromJs(bool isLocked) override;

    sptr<DisplayInfo> GetDefaultDisplayInfo() override;
    sptr<DisplayInfo> GetDisplayInfoById(DisplayId displayId) override;
    sptr<DisplayInfo> GetVisibleAreaDisplayInfoById(DisplayId displayId) override;
    sptr<DisplayInfo> GetDisplayInfoByScreen(ScreenId screenId) override;
    sptr<CutoutInfo> GetCutoutInfo(DisplayId displayId) override;
    DMError SetOrientation(ScreenId screenId, Orientation orientation) override;
    DMError SetOrientationFromWindow(ScreenId screenId, Orientation orientation, bool withAnimation);
    bool SetRotationFromWindow(ScreenId screenId, Rotation targetRotation, bool withAnimation);
    void SetGravitySensorSubscriptionEnabled();
    std::shared_ptr<Media::PixelMap> GetDisplaySnapshot(DisplayId displayId,
        DmErrorCode* errorCode, bool isUseDma, bool isCaptureFullOfScreen = false) override;
    DMError HasPrivateWindow(DisplayId id, bool& hasPrivateWindow) override;
    // colorspace, gamut
    DMError GetScreenSupportedColorGamuts(ScreenId screenId, std::vector<ScreenColorGamut>& colorGamuts) override;
    DMError GetScreenColorGamut(ScreenId screenId, ScreenColorGamut& colorGamut) override;
    DMError SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx) override;
    DMError GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap) override;
    DMError SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap) override;
    DMError SetScreenColorTransform(ScreenId screenId) override;

    DMError RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type) override;
    DMError UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type) override;
    bool WakeUpBegin(PowerStateChangeReason reason) override;
    bool WakeUpEnd() override;
    bool SuspendBegin(PowerStateChangeReason reason) override;
    bool SuspendEnd() override;
    bool SetSpecifiedScreenPower(ScreenId, ScreenPowerState, PowerStateChangeReason) override;
    bool SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason) override;
    ScreenPowerState GetScreenPower(ScreenId dmsScreenId) override;
    bool SetDisplayState(DisplayState state) override;
    void UpdateRSTree(DisplayId displayId, DisplayId parentDisplayId, std::shared_ptr<RSSurfaceNode>& surfaceNode,
        bool isAdd, bool isMultiDisplay);
    DMError AddSurfaceNodeToDisplay(DisplayId displayId,
        std::shared_ptr<RSSurfaceNode>& surfaceNode, bool onTop = true) override;
    DMError RemoveSurfaceNodeFromDisplay(DisplayId displayId, std::shared_ptr<RSSurfaceNode>& surfaceNode) override;
    DisplayState GetDisplayState(DisplayId displayId) override;
    bool TryToCancelScreenOff() override;
    bool SetScreenBrightness(uint64_t screenId, uint32_t level) override;
    uint32_t GetScreenBrightness(uint64_t screenId) override;
    void NotifyDisplayEvent(DisplayEvent event) override;
    bool SetFreeze(std::vector<DisplayId> displayIds, bool isFreeze) override;

    DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds, ScreenId& screenGroupId) override;
    DMError MakeExpand(std::vector<ScreenId> screenId, std::vector<Point> startPoints,
        ScreenId& screenGroupId) override;
    DMError StopMirror(const std::vector<ScreenId>& mirrorScreenIds) override;
    DMError StopExpand(const std::vector<ScreenId>& expandScreenIds) override;
    void RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens) override;
    sptr<ScreenInfo> GetScreenInfoById(ScreenId screenId) override;
    sptr<ScreenGroupInfo> GetScreenGroupInfoById(ScreenId screenId) override;
    ScreenId GetScreenGroupIdByScreenId(ScreenId screenId);
    DMError GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos) override;

    std::vector<DisplayId> GetAllDisplayIds() override;
    DMError SetScreenActiveMode(ScreenId screenId, uint32_t modeId) override;
    DMError SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio) override;
    DMError SetResolution(ScreenId screenId, uint32_t width, uint32_t height,
        float virtualPixelRatio) override { return DMError::DM_OK; }
    DMError GetDensityInCurResolution(ScreenId screenId,
        float& virtualPixelRatio) override { return DMError::DM_OK; }
    void RegisterDisplayChangeListener(sptr<IDisplayChangeListener> listener);
    void RegisterWindowInfoQueriedListener(const sptr<IWindowInfoQueriedListener>& listener);
    void NotifyPrivateWindowStateChanged(bool hasPrivate);
    std::vector<DisplayPhysicalResolution> GetAllDisplayPhysicalResolution() override;

private:
    DisplayManagerService();
    ~DisplayManagerService() = default;
    bool Init();
    void NotifyDisplayStateChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
        const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type);
    void NotifyScreenshot(DisplayId displayId);
    ScreenId GetScreenIdByDisplayId(DisplayId displayId) const;
    void ConfigureDisplayManagerService();
    void ConfigureWaterfallDisplayCompressionParams();

    std::recursive_mutex mutex_;
    static inline SingletonDelegator<DisplayManagerService> delegator_;
    sptr<AbstractDisplayController> abstractDisplayController_;
    sptr<AbstractScreenController> abstractScreenController_;
    sptr<DisplayPowerController> displayPowerController_;
    sptr<DisplayCutoutController> displayCutoutController_;
    sptr<IDisplayChangeListener> displayChangeListener_;
    sptr<IWindowInfoQueriedListener> windowInfoQueriedListener_;
    sptr<DisplayDumper> displayDumper_;
    AtomicMap<ScreenId, uint32_t> accessTokenIdMaps_;
    bool isAutoRotationOpen_;
    std::vector<DisplayPhysicalResolution> allDisplayPhysicalResolution_ {};
};
} // namespace OHOS::Rosen

#endif // FOUNDATION_DMSERVER_DISPLAY_MANAGER_SERVICE_H