/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_SCREEN_SESSION_MANAGER_PROXY_H
#define OHOS_ROSEN_SCREEN_SESSION_MANAGER_PROXY_H

#include "screen_session_manager_interface.h"

#include "iremote_proxy.h"

namespace OHOS {
namespace Rosen {

class ScreenSessionManagerProxy : public IRemoteProxy<IScreenSessionManager> {
public:
    explicit ScreenSessionManagerProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy(impl) {}

    ~ScreenSessionManagerProxy() = default;

    virtual sptr<DisplayInfo> GetDefaultDisplayInfo(int32_t userId = CONCURRENT_USER_ID_DEFAULT) override;
    virtual DMError SetScreenActiveMode(ScreenId screenId, uint32_t modeId) override;
    virtual DMError SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio) override;
    virtual DMError SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio) override;
    virtual DMError SetDefaultDensityDpi(ScreenId screenId, float virtualPixelRatio) override;
    virtual DMError SetResolution(ScreenId screenId, uint32_t width, uint32_t height, float virtualPixelRatio) override;
    virtual DMError GetDensityInCurResolution(ScreenId screenId, float& virtualPixelRatio) override;

    virtual DMError GetScreenColorGamut(ScreenId screenId, ScreenColorGamut& colorGamut) override;
    virtual DMError SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx) override;
    virtual DMError GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap) override;
    virtual DMError SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap) override;
    virtual DMError SetScreenColorTransform(ScreenId screenId) override;

    DMError GetPixelFormat(ScreenId screenId, GraphicPixelFormat& pixelFormat) override;
    DMError SetPixelFormat(ScreenId screenId, GraphicPixelFormat pixelFormat) override;
    DMError GetSupportedHDRFormats(ScreenId screenId, std::vector<ScreenHDRFormat>& hdrFormats) override;
    DMError GetScreenHDRFormat(ScreenId screenId, ScreenHDRFormat& hdrFormat) override;
    DMError SetScreenHDRFormat(ScreenId screenId, int32_t modeIdx) override;
    DMError GetSupportedColorSpaces(ScreenId screenId, std::vector<GraphicCM_ColorSpaceType>& colorSpaces) override;
    DMError GetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType& colorSpace) override;
    DMError SetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType colorSpace) override;

    virtual DMError RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type) override;

    virtual DMError UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type) override;

    virtual DMError RegisterDisplayAttributeAgent(std::vector<std::string>& attributes,
        const sptr<IDisplayManagerAgent>& displayManagerAgent) override;
    virtual DMError UnRegisterDisplayAttribute(const std::vector<std::string>& attributes,
        const sptr<IDisplayManagerAgent>& displayManagerAgent) override;

    virtual void NotifyDisplayEvent(DisplayEvent event) override;
    virtual bool WakeUpBegin(PowerStateChangeReason reason) override;
    virtual bool WakeUpEnd() override;
    virtual bool SuspendBegin(PowerStateChangeReason reason) override;
    virtual bool SuspendEnd() override;
    virtual ScreenId GetInternalScreenId() override;
    virtual bool SetScreenPowerById(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason) override;
    virtual bool SetSpecifiedScreenPower(ScreenId, ScreenPowerState, PowerStateChangeReason) override;
    virtual bool SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason) override;
    virtual ScreenPowerState GetScreenPower(ScreenId dmsScreenId) override;
    virtual ScreenPowerState GetScreenPower() override;
    virtual bool SetDisplayState(DisplayState state) override;
    virtual DisplayState GetDisplayState(DisplayId displayId) override;
    virtual bool TryToCancelScreenOff() override;
    ScreenId CreateVirtualScreen(VirtualScreenOption option,
        const sptr<IRemoteObject>& displayManagerAgent) override;

    virtual DMError DestroyVirtualScreen(ScreenId screenId) override;

    virtual DMError SetVirtualScreenSurface(ScreenId screenId, sptr<IBufferProducer> surface) override;

    DMError AddVirtualScreenBlockList(const std::vector<int32_t>& persistentIds) override;

    DMError RemoveVirtualScreenBlockList(const std::vector<int32_t>& persistentIds) override;

    template <DisplayManagerMessage TRANS_ID_WHITELIST>
    DMError SendVirtualScreenWhiteListRequest(ScreenId screenId, const std::vector<uint64_t>& missionIds);

    DMError AddVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds) override;

    DMError RemoveVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds) override;

    bool IsOnboardDisplay(DisplayId displayId) override;

    virtual DMError SetScreenPrivacyMaskImage(ScreenId screenId,
        const std::shared_ptr<Media::PixelMap>& privacyMaskImg) override;

    virtual DMError ResizeVirtualScreen(ScreenId screenId, uint32_t width, uint32_t height) override;

    virtual DMError SetVirtualMirrorScreenCanvasRotation(ScreenId screenId, bool autoRotate) override;

    virtual DMError SetVirtualMirrorScreenScaleMode(ScreenId screenId, ScreenScaleMode scaleMode) override;

    virtual DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
        ScreenId& screenGroupId, const RotationOption& rotationOption = {Rotation::ROTATION_0, false},
        bool forceRecord = false) override;
    virtual DMError MakeMirrorForRecord(const std::vector<ScreenId>& mainScreenIds,
        std::vector<ScreenId>& mirrorScreenIds, ScreenId& screenGroupId) override;
    virtual DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
        DMRect mainScreenRegion, ScreenId& screenGroupId) override;
    virtual DMError SetMultiScreenMode(ScreenId mainScreenId, ScreenId secondaryScreenId,
        MultiScreenMode screenMode) override;
    virtual DMError SetMultiScreenRelativePosition(MultiScreenPositionOptions mainScreenOptions,
        MultiScreenPositionOptions secondScreenOption) override;
    virtual DMError StopMirror(const std::vector<ScreenId>& mirrorScreenIds) override;
    DMError DisableMirror(bool disableOrNot) override;

    virtual DMError MakeExpand(std::vector<ScreenId> screenId, std::vector<Point> startPoint,
                               ScreenId& screenGroupId) override;
    virtual DMError StopExpand(const std::vector<ScreenId>& expandScreenIds) override;

    virtual sptr<ScreenGroupInfo> GetScreenGroupInfoById(ScreenId screenId) override;

    virtual void RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens) override;

    virtual std::shared_ptr<Media::PixelMap> GetDisplaySnapshot(DisplayId displayId,
        DmErrorCode* errorCode, bool isUseDma, bool isCaptureFullOfScreen) override;
    virtual std::vector<std::shared_ptr<Media::PixelMap>> GetDisplayHDRSnapshot(DisplayId displayId,
        DmErrorCode& errorCode, bool isUseDma, bool isCaptureFullOfScreen) override;
    virtual std::shared_ptr<Media::PixelMap> GetSnapshotByPicker(Media::Rect &rect, DmErrorCode* errorCode) override;

    virtual sptr<DisplayInfo> GetDisplayInfoById(DisplayId displayId) override;
    virtual sptr<DisplayInfo> GetVisibleAreaDisplayInfoById(DisplayId displayId) override;
    virtual sptr<DisplayInfo> GetDisplayInfoByScreen(ScreenId screenId) override;
    virtual std::vector<DisplayId> GetAllDisplayIds(int32_t userId = CONCURRENT_USER_ID_DEFAULT) override;

    virtual sptr<ScreenInfo> GetScreenInfoById(ScreenId screenId) override;

    virtual DMError GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos) override;

    virtual DMError GetScreenSupportedColorGamuts(ScreenId screenId,
        std::vector<ScreenColorGamut>& colorGamuts) override;

    virtual DMError SetOrientation(ScreenId screenId, Orientation orientation, bool isFromNapi) override;
    virtual DMError SetScreenRotationLocked(bool isLocked) override;
    virtual DMError SetScreenRotationLockedFromJs(bool isLocked) override;
    virtual DMError IsScreenRotationLocked(bool& isLocked) override;
    virtual sptr<CutoutInfo> GetCutoutInfo(DisplayId displayId) override;
    virtual sptr<CutoutInfo> GetCutoutInfo(DisplayId displayId, int32_t width,
                                           int32_t height, Rotation rotation) override;
    virtual DMError HasImmersiveWindow(ScreenId screenId, bool& immersive) override;

    virtual DMError HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow) override;
    virtual bool ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId) override;
    virtual void UpdateDisplayHookInfo(int32_t uid, bool enable, const DMHookInfo& hookInfo) override;
    void GetDisplayHookInfo(int32_t uid, DMHookInfo& hookInfo) override;
    void NotifyIsFullScreenInForceSplitMode(int32_t uid, bool isFullScreen) override;

    virtual void DumpAllScreensInfo(std::string& dumpInfo) override;
    virtual void DumpSpecialScreenInfo(ScreenId id, std::string& dumpInfo) override;
    //Fold Screen
    void SetFoldDisplayMode(const FoldDisplayMode displayMode) override;
    void SetFoldDisplayModeAsync(const FoldDisplayMode displayMode) override;
    DMError SetFoldDisplayModeFromJs(const FoldDisplayMode displayMode, std::string reason = "") override;

    void SetDisplayScale(ScreenId screenId, float scaleX, float scaleY,
        float pivotX, float pivotY) override;

    void SetFoldStatusLocked(bool locked) override;
    DMError SetFoldStatusLockedFromJs(bool locked) override;
    DMError ForceSetFoldStatusAndLock(FoldStatus targetFoldStatus) override;
    DMError RestorePhysicalFoldStatus() override;
    void SetFoldStatusExpandAndLocked(bool locked) override;

    FoldDisplayMode GetFoldDisplayMode() override;
    DMError GetPhysicalScreenIds(std::vector<ScreenId>& screenIds) override;

    bool IsFoldable() override;
    bool IsCaptured() override;

    FoldStatus GetFoldStatus() override;
    SuperFoldStatus GetSuperFoldStatus() override;
    float GetSuperRotation() override;
    void SetLandscapeLockStatus(bool isLocked) override;
    ExtendScreenConnectStatus GetExtendScreenConnectStatus() override;
    sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion() override;
    DMError GetLiveCreaseRegion(FoldCreaseRegion& region) override;
    void SetForceCloseHdr(ScreenId screenId, bool isForceCloseHdr) override;

    void SetCameraStatus(int32_t cameraStatus, int32_t cameraPosition) override;

    // unique screen
    DMError MakeUniqueScreen(const std::vector<ScreenId>& screenIds, std::vector<DisplayId>& displayIds,
        const UniqueScreenRotationOptions& rotationOptions) override;

    void SetClient(const sptr<IScreenSessionManagerClient>& client) override;
    ScreenProperty GetScreenProperty(ScreenId screenId) override;
    std::shared_ptr<RSDisplayNode> GetDisplayNode(ScreenId screenId) override;
    void UpdateScreenRotationProperty(ScreenId screenId, const RRectT<float>& bounds, float rotation,
        ScreenPropertyChangeType screenPropertyChangeType, bool isSwitchUser) override;
    void UpdateScreenDirectionInfo(ScreenId screenId, const ScreenDirectionInfo& directionInfo,
        ScreenPropertyChangeType screenPropertyChangeType, const RRect& bounds) override;
    void UpdateAvailableArea(ScreenId ScreenId, DMRect area) override;
    void UpdateSuperFoldAvailableArea(ScreenId screenId, DMRect bArea, DMRect cArea) override;
    void UpdateSuperFoldExpandAvailableArea(ScreenId screenId, DMRect area) override;
    int32_t SetScreenOffDelayTime(int32_t delay) override;
    int32_t SetScreenOnDelayTime(int32_t delay) override;
    uint32_t GetCurvedCompressionArea() override;
    ScreenProperty GetPhyScreenProperty(ScreenId screenId) override;
    void NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info) override;
    void SetScreenPrivacyState(bool hasPrivate) override;
    void SetPrivacyStateByDisplayId(std::unordered_map<DisplayId, bool>& privacyBundleDisplayId) override;
    void SetScreenPrivacyWindowList(DisplayId id, std::vector<std::string> privacyWindowList) override;
    virtual DMError GetAvailableArea(DisplayId displayId, DMRect& area) override;
    virtual DMError GetExpandAvailableArea(DisplayId displayId, DMRect& area) override;
    void NotifyFoldToExpandCompletion(bool foldToExpand) override;
    void NotifyScreenConnectCompletion(ScreenId screenId) override;
    void NotifyAodOpCompletion(AodOP op, int32_t result) override;
    void RecordEventFromScb(std::string description, bool needRecordEvent) override;
    void SwitchUser() override;

    VirtualScreenFlag GetVirtualScreenFlag(ScreenId screenId) override;
    DMError SetVirtualScreenFlag(ScreenId screenId, VirtualScreenFlag screenFlag) override;
    DeviceScreenConfig GetDeviceScreenConfig() override;
    DMError SetVirtualScreenRefreshRate(ScreenId screenId, uint32_t refreshInterval) override;
    void SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList,
        std::vector<uint64_t> surfaceIdList = {}, std::vector<uint8_t> typeBlackList = {}) override;
    void SetVirtualDisplayMuteFlag(ScreenId screenId, bool muteFlag) override;
    void DisablePowerOffRenderControl(ScreenId screenId) override;
    DMError ProxyForFreeze(const std::set<int32_t>& pidList, bool isProxy) override;
    DMError ResetAllFreezeStatus() override;
    std::vector<DisplayPhysicalResolution> GetAllDisplayPhysicalResolution() override;
    DMError GetDisplayCapability(std::string& capabilitInfo) override;
    bool SetVirtualScreenStatus(ScreenId screenId, VirtualScreenStatus screenStatus) override;
    DMError SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid,
        std::vector<uint64_t>& windowIdList) override;
    DMError SetVirtualScreenMaxRefreshRate(ScreenId id, uint32_t refreshRate,
        uint32_t& actualRefreshRate) override;
    std::shared_ptr<Media::PixelMap> GetScreenCapture(const CaptureOption& captureOption,
        DmErrorCode* errorCode = nullptr) override;
    std::shared_ptr<Media::PixelMap> GetDisplaySnapshotWithOption(const CaptureOption& captureOption,
        DmErrorCode* errorCode) override;
    std::vector<std::shared_ptr<Media::PixelMap>> GetDisplayHDRSnapshotWithOption(const CaptureOption& captureOption,
        DmErrorCode& errorCode) override;
    sptr<DisplayInfo> GetPrimaryDisplayInfo() override;
    DisplayId GetPrimaryDisplayId() override;
    ScreenCombination GetScreenCombination(ScreenId screenId) override;
    DMError SetScreenSkipProtectedWindow(const std::vector<ScreenId>& screenIds, bool isEnable) override;
    bool GetIsRealScreen(ScreenId screenId) override;
    void SetDefaultMultiScreenModeWhenSwitchUser() override;
    void NotifyExtendScreenCreateFinish() override;
    void NotifyExtendScreenDestroyFinish() override;
    void NotifyScreenMaskAppear() override;
    DMError GetScreenAreaOfDisplayArea(DisplayId displayId, const DMRect& displayArea,
        ScreenId& screenId, DMRect& screenArea) override;
    DMError GetBrightnessInfo(DisplayId displayId, ScreenBrightnessInfo& brightnessInfo) override;
    DMError GetSupportsInput(DisplayId displayId, bool& supportsInput) override;
    DMError SetSupportsInput(DisplayId displayId, bool supportsInput) override;
    DMError GetBundleName(DisplayId displayId, std::string& bundleName) override;
    DMError SetPrimaryDisplaySystemDpi(float dpi) override;
    DMError SetVirtualScreenAutoRotation(ScreenId screenId, bool enable) override;
    DMError SetScreenPrivacyWindowTagSwitch(ScreenId screenId, const std::vector<std::string>& privacyWindowTag,
        bool enable) override;
    void NotifySwitchUserAnimationFinish() override;
    DMError SyncScreenPropertyChangedToServer(ScreenId screenId, const ScreenProperty& screenProperty) override;
    DMError GetRoundedCorner(DisplayId displayId, int& radius) override;

private:
    static inline BrokerDelegator<ScreenSessionManagerProxy> delegator_;
};

} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_SCREEN_SESSION_MANAGER_PROXY_H
