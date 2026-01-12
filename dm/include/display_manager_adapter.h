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

#ifndef FOUNDATION_DM_DISPLAY_MANAGER_ADAPTER_H
#define FOUNDATION_DM_DISPLAY_MANAGER_ADAPTER_H

#include <mutex>
#include <surface.h>

#include "display.h"
#include "dm_common.h"
#include "fold_screen_info.h"
#include "idisplay_manager.h"
#include "screen.h"
#include "screen_group.h"
#include "singleton_delegator.h"
#include "zidl/idisplay_manager_agent.h"
#include "zidl/screen_session_manager_interface.h"

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
    sptr<IScreenSessionManager> screenSessionManagerServiceProxy_ = nullptr;
    sptr<IDisplayManager> displayManagerServiceProxy_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> dmsDeath_ = nullptr;
    bool isProxyValid_ = false;
    static inline DMError ConvertToDMError(ErrCode errCode, int32_t dmError);
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
    virtual sptr<DisplayInfo> GetDefaultDisplayInfo(int32_t userId = CONCURRENT_USER_ID_DEFAULT);
    virtual sptr<DisplayInfo> GetDisplayInfoByScreenId(ScreenId screenId);
    virtual std::vector<DisplayId> GetAllDisplayIds(int32_t userId = CONCURRENT_USER_ID_DEFAULT);
    virtual std::shared_ptr<Media::PixelMap> GetDisplaySnapshot(DisplayId displayId,
        DmErrorCode* errorCode = nullptr, bool isUseDma = false, bool isCaptureFullOfScreen = false);
    virtual std::vector<std::shared_ptr<Media::PixelMap>> GetDisplayHDRSnapshot(DisplayId displayId,
        DmErrorCode& errorCode, bool isUseDma = false, bool isCaptureFullOfScreen = false);
    virtual std::shared_ptr<Media::PixelMap> GetSnapshotByPicker(Media::Rect &rect, DmErrorCode* errorCode = nullptr);
    virtual DMError HasImmersiveWindow(ScreenId screenId, bool& immersive);
    virtual DMError HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow);
    virtual bool WakeUpBegin(PowerStateChangeReason reason);
    virtual bool WakeUpEnd();
    virtual bool SuspendBegin(PowerStateChangeReason reason);
    virtual bool SuspendEnd();
    virtual ScreenId GetInternalScreenId();
    virtual bool SetScreenPowerById(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason);
    virtual bool SetDisplayState(DisplayState state);
    virtual DisplayState GetDisplayState(DisplayId displayId);
    virtual bool TryToCancelScreenOff();
    virtual void NotifyDisplayEvent(DisplayEvent event);
    virtual bool SetFreeze(std::vector<DisplayId> displayIds, bool isFreeze);
    virtual sptr<DisplayInfo> GetDisplayInfo(DisplayId displayId);
    virtual sptr<DisplayInfo> GetVisibleAreaDisplayInfoById(DisplayId displayId);
    virtual DMError GetExpandAvailableArea(DisplayId displayId, DMRect& area);
    virtual DMError GetAvailableArea(DisplayId displayId, DMRect& area);
    virtual sptr<CutoutInfo> GetCutoutInfo(DisplayId displayId, int32_t width, int32_t height, Rotation rotation);
    virtual DMError AddSurfaceNodeToDisplay(DisplayId displayId, std::shared_ptr<class RSSurfaceNode>& surfaceNode);
    virtual DMError RemoveSurfaceNodeFromDisplay(DisplayId displayId,
        std::shared_ptr<class RSSurfaceNode>& surfaceNode);
    virtual bool ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId);
    virtual bool IsFoldable();
    virtual bool IsCaptured();
    virtual FoldStatus GetFoldStatus();
    virtual FoldDisplayMode GetFoldDisplayMode();
    virtual void SetFoldDisplayMode(const FoldDisplayMode);
    virtual void SetFoldDisplayModeAsync(const FoldDisplayMode);
    virtual DMError SetFoldDisplayModeFromJs(const FoldDisplayMode, std::string reason = "");
    virtual void SetDisplayScale(ScreenId screenId, float scaleX, float scaleY, float pivotX, float pivotY);
    virtual void SetFoldStatusLocked(bool locked);
    virtual DMError SetFoldStatusLockedFromJs(bool locked);
    virtual DMError ForceSetFoldStatusAndLock(FoldStatus targetFoldStatus);
    virtual DMError RestorePhysicalFoldStatus();
    virtual sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion();
    virtual DMError GetLiveCreaseRegion(FoldCreaseRegion& region);
    virtual void SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList,
        std::vector<uint64_t> surfaceIdList = {}, std::vector<uint8_t> typeBlackList = {});
    virtual bool IsOnboardDisplay(DisplayId displayId);
    virtual void SetVirtualDisplayMuteFlag(ScreenId screenId, bool muteFlag);
    virtual void DisablePowerOffRenderControl(ScreenId screenId);
    virtual DMError ProxyForFreeze(const std::set<int32_t>& pidList, bool isProxy);
    virtual DMError ResetAllFreezeStatus();
    virtual std::vector<DisplayPhysicalResolution> GetAllDisplayPhysicalResolution();
    virtual DMError GetDisplayCapability(std::string& capabilitInfo);
    virtual DMError SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid,
        std::vector<uint64_t>& windowIdList);
    virtual std::shared_ptr<Media::PixelMap> GetScreenCapture(const CaptureOption& captureOption,
        DmErrorCode* errorCode = nullptr);
    virtual std::shared_ptr<Media::PixelMap> GetDisplaySnapshotWithOption(const CaptureOption& captureOption,
        DmErrorCode* errorCode = nullptr);
    virtual std::vector<std::shared_ptr<Media::PixelMap>> GetDisplayHDRSnapshotWithOption(
        const CaptureOption& captureOption, DmErrorCode& errorCode);
    virtual sptr<DisplayInfo> GetPrimaryDisplayInfo();
    virtual DMError GetScreenAreaOfDisplayArea(DisplayId displayId, const DMRect& displayArea,
        ScreenId& screenId, DMRect& screenArea);
    virtual DMError GetBrightnessInfo(DisplayId displayId, ScreenBrightnessInfo& brightnessInfo);
    virtual bool SetVirtualScreenAsDefault(ScreenId screenId);
    virtual DisplayId GetPrimaryDisplayId();
    virtual DMError GetSupportsInput(DisplayId displayId, bool& supportsInput);
    virtual DMError SetSupportsInput(DisplayId displayId, bool supportsInput);
    virtual DMError GetRoundedCorner(std::vector<RoundedCorner>& roundedCorner, DisplayId displayId,
        int32_t width, int32_t height);
    virtual DMError GetBundleName(DisplayId displayId, std::string& bundleName);
    virtual DMError RegisterDisplayAttributeAgent(std::vector<std::string>& attributes,
        const sptr<IDisplayManagerAgent> displayManagerAgent);
    virtual DMError UnRegisterDisplayAttribute(const std::vector<std::string>& attributes,
        const sptr<IDisplayManagerAgent> displayManagerAgent);
    
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
    virtual DMError AddVirtualScreenBlockList(const std::vector<int32_t>& persistentIds);
    virtual DMError RemoveVirtualScreenBlockList(const std::vector<int32_t>& persistentIds);
    virtual DMError AddVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds);
    virtual DMError RemoveVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds);
    virtual DMError SetScreenPrivacyMaskImage(ScreenId screenId,
        const std::shared_ptr<Media::PixelMap>& privacyMaskImg);
    virtual DMError SetVirtualMirrorScreenCanvasRotation(ScreenId screenId, bool canvasRotation);
    virtual DMError SetVirtualMirrorScreenScaleMode(ScreenId screenId, ScreenScaleMode scaleMode);
    virtual bool SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason);
    virtual bool SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason);
    virtual ScreenPowerState GetScreenPower(ScreenId dmsScreenId);
    virtual DMError SetOrientation(ScreenId screenId, Orientation orientation, bool isFromNapi);
    virtual sptr<ScreenGroupInfo> GetScreenGroupInfoById(ScreenId screenId);
    virtual DMError GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos);
    virtual DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId, ScreenId& screenGroupId,
        const RotationOption& rotationOption = {Rotation::ROTATION_0, false});
    virtual DMError MakeMirrorForRecord(const std::vector<ScreenId>& mainScreenIds,
        std::vector<ScreenId>& mirrorScreenIds, ScreenId& screenGroupId);
    virtual DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId, DMRect mainScreenRegion,
        ScreenId& screenGroupId);
    virtual DMError SetMultiScreenMode(ScreenId mainScreenId, ScreenId secondaryScreenId,
        MultiScreenMode screenMode);
    virtual DMError SetMultiScreenRelativePosition(MultiScreenPositionOptions mainScreenOptions,
        MultiScreenPositionOptions secondScreenOption);
    virtual DMError MakeExpand(std::vector<ScreenId> screenId, std::vector<Point> startPoint, ScreenId& screenGroupId);
    virtual DMError StopMirror(const std::vector<ScreenId>& mirrorScreenIds);
    virtual DMError StopExpand(const std::vector<ScreenId>& expandScreenIds);
    virtual DMError DisableMirror(bool disableOrNot);
    virtual void RemoveVirtualScreenFromGroup(std::vector<ScreenId>);
    virtual DMError SetScreenActiveMode(ScreenId screenId, uint32_t modeId);
    virtual sptr<ScreenInfo> GetScreenInfo(ScreenId screenId);
    virtual DMError SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio);
    virtual DMError SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio);
    virtual DMError SetDefaultDensityDpi(ScreenId screenId, float virtualPixelRatio);
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
    virtual DMError MakeUniqueScreen(const std::vector<ScreenId>& screenIds, std::vector<DisplayId>& displayIds,
        const UniqueScreenRotationOptions& rotationOptions);
    virtual VirtualScreenFlag GetVirtualScreenFlag(ScreenId screenId);
    virtual DMError SetVirtualScreenFlag(ScreenId screenId, VirtualScreenFlag screenFlag);
    virtual DMError SetVirtualScreenRefreshRate(ScreenId screenId, uint32_t refreshInterval);
    virtual bool SetVirtualScreenStatus(ScreenId screenId, VirtualScreenStatus screenStatus);
    virtual DMError SetVirtualScreenMaxRefreshRate(ScreenId id, uint32_t refreshRate,
        uint32_t& actualRefreshRate);
    virtual ScreenPowerState GetScreenPower();
    virtual void SetFoldStatusExpandAndLocked(bool locked);
    virtual DMError SetScreenSkipProtectedWindow(const std::vector<ScreenId>& screenIds, bool isEnable);
    virtual DMError SetVirtualScreenAutoRotation(ScreenId screenId, bool enable);
    virtual DMError SetScreenPrivacyWindowTagSwitch(ScreenId screenId, const std::vector<std::string>& privacyWindowTag,
        bool enable);
private:
    static inline SingletonDelegator<ScreenManagerAdapter> delegator;
};
} // namespace OHOS::Rosen
#endif // FOUNDATION_DM_DISPLAY_MANAGER_ADAPTER_H
