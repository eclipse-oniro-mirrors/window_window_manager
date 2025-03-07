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

#ifndef FOUNDATION_DM_DISPLAY_MANAGER_ADAPTER_H
#define FOUNDATION_DM_DISPLAY_MANAGER_ADAPTER_H

#include <map>
#include <mutex>
#include <surface.h>

#include "display.h"
#include "screen.h"
#include "screen_group.h"
#include "dm_common.h"
#include "display_manager_interface.h"
#include "fold_screen_info.h"
#include "singleton_delegator.h"

namespace OHOS::Rosen {
class BaseAdapter {
public:
    virtual ~BaseAdapter();
    virtual DMError RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type);
    virtual DMError UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type);
    virtual void Clear();
protected:
    bool InitDMSProxy();
    std::recursive_mutex mutex_;
    sptr<IDisplayManager> displayManagerServiceProxy_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> dmsDeath_ = nullptr;
    bool isProxyValid_ { false };
};

class DMSDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit DMSDeathRecipient(BaseAdapter& adapter);
    virtual void OnRemoteDied(const wptr<IRemoteObject>& wptrDeath) override;
private:
    BaseAdapter& adapter_;
};

class DisplayManagerAdapter : public BaseAdapter {
WM_DECLARE_SINGLE_INSTANCE(DisplayManagerAdapter);
public:
    virtual sptr<DisplayInfo> GetDefaultDisplayInfo();
    virtual sptr<DisplayInfo> GetDisplayInfoByScreenId(ScreenId screenId);
    virtual std::vector<DisplayId> GetAllDisplayIds();
    virtual std::shared_ptr<Media::PixelMap> GetDisplaySnapshot(DisplayId displayId,
        DmErrorCode* errorCode = nullptr, bool isUseDma = false);
    virtual std::shared_ptr<Media::PixelMap> GetSnapshotByPicker(Media::Rect &rect, DmErrorCode* errorCode = nullptr);
    virtual DMError HasImmersiveWindow(bool& immersive);
    virtual DMError HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow);
    virtual bool WakeUpBegin(PowerStateChangeReason reason);
    virtual bool WakeUpEnd();
    virtual bool SuspendBegin(PowerStateChangeReason reason);
    virtual bool SuspendEnd();
    virtual bool SetDisplayState(DisplayState state);
    virtual DisplayState GetDisplayState(DisplayId displayId);
    virtual bool TryToCancelScreenOff();
    virtual void NotifyDisplayEvent(DisplayEvent event);
    virtual bool SetFreeze(std::vector<DisplayId> displayIds, bool isFreeze);
    virtual sptr<DisplayInfo> GetDisplayInfo(DisplayId displayId);
    virtual DMError GetAvailableArea(DisplayId displayId, DMRect& area);
    virtual sptr<CutoutInfo> GetCutoutInfo(DisplayId displayId);
    virtual DMError AddSurfaceNodeToDisplay(DisplayId displayId, std::shared_ptr<class RSSurfaceNode>& surfaceNode);
    virtual DMError RemoveSurfaceNodeFromDisplay(DisplayId displayId,
        std::shared_ptr<class RSSurfaceNode>& surfaceNode);
    virtual bool ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId);
    virtual bool IsFoldable();
    virtual bool IsCaptured();
    virtual FoldStatus GetFoldStatus();
    virtual FoldDisplayMode GetFoldDisplayMode();
    virtual void SetFoldDisplayMode(const FoldDisplayMode);
    virtual DMError SetFoldDisplayModeFromJs(const FoldDisplayMode, std::string reason = "");
    virtual void SetDisplayScale(ScreenId screenId, float scaleX, float scaleY, float pivotX, float pivotY);
    virtual void SetFoldStatusLocked(bool locked);
    virtual DMError SetFoldStatusLockedFromJs(bool locked);
    virtual sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion();
    virtual DMError ProxyForFreeze(const std::set<int32_t>& pidList, bool isProxy);
    virtual DMError ResetAllFreezeStatus();
    virtual void SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList,
        std::vector<uint64_t> surfaceIdList = {});
    virtual void DisablePowerOffRenderControl(ScreenId screenId);
    virtual std::vector<DisplayPhysicalResolution> GetAllDisplayPhysicalResolution();
    virtual DMError SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid,
        std::vector<uint64_t>& windowIdList);
    virtual std::shared_ptr<Media::PixelMap> GetScreenCapture(const CaptureOption& captureOption,
        DmErrorCode* errorCode = nullptr);
    virtual std::shared_ptr<Media::PixelMap> GetDisplaySnapshotWithOption(const CaptureOption& captureOption,
        DmErrorCode* errorCode = nullptr);
    virtual sptr<DisplayInfo> GetPrimaryDisplayInfo();

private:
    static inline SingletonDelegator<DisplayManagerAdapter> delegator;
};

class ScreenManagerAdapter : public BaseAdapter {
WM_DECLARE_SINGLE_INSTANCE(ScreenManagerAdapter);
public:
    virtual ScreenId CreateVirtualScreen(VirtualScreenOption option,
        const sptr<IDisplayManagerAgent>& displayManagerAgent);
    virtual DMError DestroyVirtualScreen(ScreenId screenId);
    virtual DMError SetVirtualScreenSurface(ScreenId screenId, sptr<Surface> surface);
    virtual DMError SetVirtualMirrorScreenCanvasRotation(ScreenId screenId, bool canvasRotation);
    virtual DMError SetVirtualMirrorScreenScaleMode(ScreenId screenId, ScreenScaleMode scaleMode);
    virtual bool SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason);
    virtual bool SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason);
    virtual ScreenPowerState GetScreenPower(ScreenId dmsScreenId);
    virtual DMError SetOrientation(ScreenId screenId, Orientation orientation);
    virtual sptr<ScreenGroupInfo> GetScreenGroupInfoById(ScreenId screenId);
    virtual DMError GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos);
    virtual DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId, ScreenId& screenGroupId);
    virtual DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId, DMRect mainScreenRegion,
        ScreenId& screenGroupId);
    virtual DMError MakeExpand(std::vector<ScreenId> screenId, std::vector<Point> startPoint, ScreenId& screenGroupId);
    virtual DMError StopMirror(const std::vector<ScreenId>& mirrorScreenIds);
    virtual DMError StopExpand(const std::vector<ScreenId>& expandScreenIds);
    virtual DMError DisableMirror(bool disableOrNot);
    virtual void RemoveVirtualScreenFromGroup(std::vector<ScreenId>);
    virtual DMError SetScreenActiveMode(ScreenId screenId, uint32_t modeId);
    virtual sptr<ScreenInfo> GetScreenInfo(ScreenId screenId);
    virtual DMError SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio);
    virtual DMError SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio);
    virtual DMError SetResolution(ScreenId screenId, uint32_t width, uint32_t height, float virtualPixelRatio);
    virtual DMError GetDensityInCurResolution(ScreenId screenId, float& virtualPixelRatio);
    virtual DMError ResizeVirtualScreen(ScreenId screenId, uint32_t width, uint32_t height);
    virtual DMError SetScreenRotationLocked(bool isLocked);
    virtual DMError SetScreenRotationLockedFromJs(bool isLocked);
    virtual DMError IsScreenRotationLocked(bool& isLocked);
    // colorspace, gamut
    virtual DMError GetScreenSupportedColorGamuts(ScreenId screenId, std::vector<ScreenColorGamut>& colorGamuts);
    virtual DMError GetScreenColorGamut(ScreenId screenId, ScreenColorGamut& colorGamut);
    virtual DMError SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx);
    virtual DMError GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap);
    virtual DMError SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap);
    virtual DMError SetScreenColorTransform(ScreenId screenId);
    virtual DMError GetPixelFormat(ScreenId screenId, GraphicPixelFormat& pixelFormat);
    virtual DMError SetPixelFormat(ScreenId screenId, GraphicPixelFormat pixelFormat);
    virtual DMError GetSupportedHDRFormats(ScreenId screenId, std::vector<ScreenHDRFormat>& hdrFormats);
    virtual DMError GetScreenHDRFormat(ScreenId screenId, ScreenHDRFormat& hdrFormat);
    virtual DMError SetScreenHDRFormat(ScreenId screenId, int32_t modeIdx);
    virtual DMError GetSupportedColorSpaces(ScreenId screenId, std::vector<GraphicCM_ColorSpaceType>& colorSpaces);
    virtual DMError GetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType& colorSpace);
    virtual DMError SetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType colorSpace);
    virtual DMError GetSupportedHDRFormats(ScreenId screenId, std::vector<uint32_t>& hdrFormats);
    virtual DMError GetSupportedColorSpaces(ScreenId screenId, std::vector<uint32_t>& colorSpaces);
    // unique screen
    virtual DMError MakeUniqueScreen(const std::vector<ScreenId>& screenIds);
    virtual VirtualScreenFlag GetVirtualScreenFlag(ScreenId screenId);
    virtual DMError SetVirtualScreenFlag(ScreenId screenId, VirtualScreenFlag screenFlag);
    virtual DMError SetVirtualScreenRefreshRate(ScreenId screenId, uint32_t refreshInterval);
    virtual DMError SetVirtualScreenMaxRefreshRate(ScreenId id, uint32_t refreshRate,
        uint32_t& actualRefreshRate);
    virtual ScreenPowerState GetScreenPower();
private:
    static inline SingletonDelegator<ScreenManagerAdapter> delegator;
};
} // namespace OHOS::Rosen
#endif // FOUNDATION_DM_DISPLAY_MANAGER_ADAPTER_H
