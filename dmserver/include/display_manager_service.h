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

#ifndef FOUNDATION_DMSERVER_DISPLAY_MANAGER_SERVICE_H
#define FOUNDATION_DMSERVER_DISPLAY_MANAGER_SERVICE_H

#include <map>
#include <mutex>

#include <surface.h>
#include <system_ability.h>
#include <ui/rs_display_node.h>

#include "abstract_display.h"
#include "abstract_display_controller.h"
#include "abstract_screen_controller.h"
#include "atomic_map.h"
#include "display_change_listener.h"
#include "display_cutout_controller.h"
#include "display_dumper.h"
#include "display_manager_stub.h"
#include "display_power_controller.h"
#include "dm_common.h"
#include "screen.h"
#include "singleton_delegator.h"
#include "window_info_queried_listener.h"

namespace OHOS::Rosen {
class DisplayManagerService {
WM_DECLARE_SINGLE_INSTANCE_BASE(DisplayManagerService);

public:
    DisplayManagerService();
    bool Init();
    int Dump(int fd, const std::vector<std::u16string>& args);
    ScreenId CreateVirtualScreen(VirtualScreenOption option,
        const sptr<IRemoteObject>& displayManagerAgent);
    DMError DestroyVirtualScreen(ScreenId screenId);
    DMError SetVirtualScreenSurface(ScreenId screenId, sptr<IBufferProducer> surface);
    DMError IsScreenRotationLocked(bool& isLocked);
    DMError SetScreenRotationLocked(bool isLocked);
    DMError SetScreenRotationLockedFromJs(bool isLocked);

    sptr<DisplayInfo> GetDefaultDisplayInfo();
    sptr<DisplayInfo> GetDisplayInfoById(DisplayId displayId);
    sptr<DisplayInfo> GetVisibleAreaDisplayInfoById(DisplayId displayId);
    sptr<DisplayInfo> GetDisplayInfoByScreen(ScreenId screenId);
    sptr<CutoutInfo> GetCutoutInfo(DisplayId displayId);
    DMError SetOrientation(ScreenId screenId, Orientation orientation);
    DMError SetOrientationFromWindow(ScreenId screenId, Orientation orientation, bool withAnimation);
    bool SetRotationFromWindow(ScreenId screenId, Rotation targetRotation, bool withAnimation);
    void SetGravitySensorSubscriptionEnabled();
    std::shared_ptr<Media::PixelMap> GetDisplaySnapshot(DisplayId displayId,
        DmErrorCode* errorCode, bool isUseDma, bool isCaptureFullOfScreen = false);
    DMError HasPrivateWindow(DisplayId id, bool& hasPrivateWindow);
    // colorspace, gamut
    DMError GetScreenSupportedColorGamuts(ScreenId screenId, std::vector<ScreenColorGamut>& colorGamuts);
    DMError GetScreenColorGamut(ScreenId screenId, ScreenColorGamut& colorGamut);
    DMError SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx);
    DMError GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap);
    DMError SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap);
    DMError SetScreenColorTransform(ScreenId screenId);

    DMError RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type);
    DMError UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type);
    bool WakeUpBegin(PowerStateChangeReason reason);
    bool WakeUpEnd();
    bool SuspendBegin(PowerStateChangeReason reason);
    bool SuspendEnd();
    bool SetSpecifiedScreenPower(ScreenId, ScreenPowerState, PowerStateChangeReason);
    bool SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason);
    ScreenPowerState GetScreenPower(ScreenId dmsScreenId);
    bool SetDisplayState(DisplayState state);
    void UpdateRSTree(DisplayId displayId, DisplayId parentDisplayId, std::shared_ptr<RSSurfaceNode>& surfaceNode,
        bool isAdd, bool isMultiDisplay);
    DMError AddSurfaceNodeToDisplay(DisplayId displayId,
        std::shared_ptr<RSSurfaceNode>& surfaceNode, bool onTop = true);
    DMError RemoveSurfaceNodeFromDisplay(DisplayId displayId, std::shared_ptr<RSSurfaceNode>& surfaceNode);
    DisplayState GetDisplayState(DisplayId displayId);
    bool TryToCancelScreenOff();
    bool SetScreenBrightness(uint64_t screenId, uint32_t level);
    uint32_t GetScreenBrightness(uint64_t screenId);
    void NotifyDisplayEvent(DisplayEvent event);
    bool SetFreeze(std::vector<DisplayId> displayIds, bool isFreeze);

    DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds, ScreenId& screenGroupId);
    DMError MakeExpand(std::vector<ScreenId> screenId, std::vector<Point> startPoints,
        ScreenId& screenGroupId);
    DMError StopMirror(const std::vector<ScreenId>& mirrorScreenIds);
    DMError StopExpand(const std::vector<ScreenId>& expandScreenIds);
    void RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens);
    sptr<ScreenInfo> GetScreenInfoById(ScreenId screenId);
    sptr<ScreenGroupInfo> GetScreenGroupInfoById(ScreenId screenId);
    ScreenId GetScreenGroupIdByScreenId(ScreenId screenId);
    DMError GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos);

    std::vector<DisplayId> GetAllDisplayIds();
    DMError SetScreenActiveMode(ScreenId screenId, uint32_t modeId);
    DMError SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio);
    DMError SetResolution(ScreenId screenId, uint32_t width, uint32_t height,
        float virtualPixelRatio) { return DMError::DM_OK; }
    DMError GetDensityInCurResolution(ScreenId screenId,
        float& virtualPixelRatio) { return DMError::DM_OK; }
    void RegisterDisplayChangeListener(sptr<IDisplayChangeListener> listener);
    void RegisterWindowInfoQueriedListener(const sptr<IWindowInfoQueriedListener>& listener);
    void NotifyPrivateWindowStateChanged(bool hasPrivate);
    std::vector<DisplayPhysicalResolution> GetAllDisplayPhysicalResolution();
    bool SetVirtualScreenAsDefault(ScreenId screenId);

private:
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
