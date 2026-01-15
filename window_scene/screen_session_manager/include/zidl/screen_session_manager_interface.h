/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_SCREEN_SESSION_MANAGER_INTERFACE_H
#define OHOS_ROSEN_SCREEN_SESSION_MANAGER_INTERFACE_H

#include <ui/rs_display_node.h>

#include "display_manager_interface_code.h"
#include "dm_common.h"
#include "fold_screen_info.h"
#include "interfaces/include/ws_common.h"
#include "screen.h"
#include "screen_group_info.h"
#include "session/screen/include/screen_property.h"
#include "session/screen/include/screen_session.h"
#include "window_manager_hilog.h"
#include "zidl/idisplay_manager_agent.h"
#include "zidl/screen_session_manager_client_interface.h"

namespace OHOS {
namespace Rosen {
template<typename T>
class RRectT;

class IScreenSessionManager : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.IScreenSessionManager");

    virtual sptr<DisplayInfo> GetDefaultDisplayInfo(int32_t userId = CONCURRENT_USER_ID_DEFAULT) { return nullptr; }
    virtual sptr<DisplayInfo> GetDisplayInfoById(DisplayId displayId) { return nullptr; }
    virtual sptr<DisplayInfo> GetVisibleAreaDisplayInfoById(DisplayId displayId) { return nullptr; }
    virtual sptr<DisplayInfo> GetDisplayInfoByScreen(ScreenId screenId) {return nullptr; }
    virtual DMError HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow) { return DMError::DM_OK; }
    virtual bool ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId) { return true; }
    virtual void UpdateDisplayHookInfo(int32_t uid, bool enable, const DMHookInfo& hookInfo) {}
    virtual void GetDisplayHookInfo(int32_t uid, DMHookInfo& hookInfo) {}
    virtual void NotifyIsFullScreenInForceSplitMode(int32_t uid, bool isFullScreen) {}

    virtual ScreenId CreateVirtualScreen(VirtualScreenOption option,
        const sptr<IRemoteObject>& displayManagerAgent) { return SCREEN_ID_INVALID; }
    virtual DMError DestroyVirtualScreen(ScreenId screenId, bool isCallingByThirdParty = false)
        { return DMError::DM_OK; }
    virtual DMError SetVirtualScreenSurface(ScreenId screenId, sptr<IBufferProducer> surface)
    {
        return DMError::DM_OK;
    }

    virtual DMError AddVirtualScreenBlockList(const std::vector<int32_t> &persistentIds)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual DMError RemoveVirtualScreenBlockList(const std::vector<int32_t> &persistentIds)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual DMError AddVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual DMError RemoveVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual DMError SetScreenPrivacyMaskImage(ScreenId screenId,
        const std::shared_ptr<Media::PixelMap>& privacyMaskImg)
    {
        return DMError::DM_OK;
    }
    virtual DMError IsOnboardDisplay(DisplayId displayId, bool& isOnboardDisplay)
    {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    virtual DMError SetVirtualMirrorScreenCanvasRotation(ScreenId screenId, bool autoRotate)
    {
        return DMError::DM_OK;
    }

    virtual DMError SetVirtualMirrorScreenScaleMode(ScreenId screenId, ScreenScaleMode scaleMode)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual DMError SetOrientation(ScreenId screenId, Orientation orientation, bool isFromNapi)
    {
        return DMError::DM_OK;
    }
    virtual std::shared_ptr<Media::PixelMap> GetDisplaySnapshot(DisplayId displayId, DmErrorCode* errorCode = nullptr,
        bool isUseDma = false, bool isCaptureFullOfScreen = false) { return nullptr; }
    virtual std::vector<std::shared_ptr<Media::PixelMap>> GetDisplayHDRSnapshot(
        DisplayId displayId, DmErrorCode& errorCode, bool isUseDma = false,
        bool isCaptureFullOfScreen = false) { return { nullptr, nullptr }; }
    virtual std::shared_ptr<Media::PixelMap> GetSnapshotByPicker(Media::Rect &rect,
        DmErrorCode* errorCode = nullptr)
    {
        *errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
        return nullptr;
    }
    virtual DMError SetScreenRotationLocked(bool isLocked) { return DMError::DM_OK; }
    virtual DMError SetScreenRotationLockedFromJs(bool isLocked) { return DMError::DM_OK; }
    virtual DMError IsScreenRotationLocked(bool& isLocked) { return DMError::DM_OK; }

    // colorspace, gamut
    virtual DMError GetScreenSupportedColorGamuts(ScreenId screenId,
        std::vector<ScreenColorGamut>& colorGamuts)
    {
        return DMError::DM_OK;
    }
    virtual DMError GetScreenColorGamut(ScreenId screenId, ScreenColorGamut& colorGamut)
    {
        return DMError::DM_OK;
    }
    virtual DMError SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx) { return DMError::DM_OK; }
    virtual DMError GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap) { return DMError::DM_OK; }
    virtual DMError SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap) { return DMError::DM_OK; }
    virtual DMError SetScreenColorTransform(ScreenId screenId) { return DMError::DM_OK; }

    virtual DMError GetPixelFormat(ScreenId screenId, GraphicPixelFormat &pixelFormat)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual DMError SetPixelFormat(ScreenId screenId, GraphicPixelFormat pixelFormat)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual DMError GetSupportedHDRFormats(ScreenId screenId,
        std::vector<ScreenHDRFormat> &hdrFormats)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual DMError GetScreenHDRFormat(ScreenId screenId, ScreenHDRFormat &hdrFormat)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual DMError SetScreenHDRFormat(ScreenId screenId, int32_t modeIdx)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual DMError GetSupportedColorSpaces(ScreenId screenId,
        std::vector<GraphicCM_ColorSpaceType> &colorSpaces)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual DMError GetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType &colorSpace)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual DMError SetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType colorSpace)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual DMError RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type) { return DMError::DM_OK; }
    virtual DMError UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type) { return DMError::DM_OK; }
    virtual DMError RegisterDisplayAttributeAgent(std::vector<std::string>& attributes,
        const sptr<IDisplayManagerAgent>& displayManagerAgent) { return DMError::DM_OK; }
    virtual DMError UnRegisterDisplayAttribute(const std::vector<std::string>& attributes,
        const sptr<IDisplayManagerAgent>& displayManagerAgent) { return DMError::DM_OK; }

    virtual bool WakeUpBegin(PowerStateChangeReason reason) { return false; }
    virtual bool WakeUpEnd() { return false; }
    virtual bool SuspendBegin(PowerStateChangeReason reason) { return false; }
    virtual bool SuspendEnd() { return false; }
    virtual ScreenId GetInternalScreenId() { return SCREEN_ID_INVALID; }
    virtual bool SetScreenPowerById(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason)
    {
        return false;
    }
    virtual bool SetSpecifiedScreenPower(ScreenId, ScreenPowerState, PowerStateChangeReason) { return false; }
    virtual bool SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason) { return false; }
    virtual ScreenPowerState GetScreenPower(ScreenId dmsScreenId) { return ScreenPowerState::INVALID_STATE; }
    virtual ScreenPowerState GetScreenPower() { return ScreenPowerState::INVALID_STATE; }
    virtual void SyncScreenPowerState(ScreenPowerState state) { }
    virtual bool SetDisplayState(DisplayState state) { return false; }
    virtual DisplayState GetDisplayState(DisplayId displayId) {return DisplayState::UNKNOWN; }
    virtual bool TryToCancelScreenOff() { return false; }
    virtual bool SetScreenBrightness(uint64_t screenId, uint32_t level) { return false; }
    virtual uint32_t GetScreenBrightness(uint64_t screenId) { return 0; }
    virtual std::vector<DisplayId> GetAllDisplayIds(int32_t userId = CONCURRENT_USER_ID_DEFAULT)
    {
        return std::vector<DisplayId>{};
    }
    virtual sptr<CutoutInfo> GetCutoutInfo(DisplayId displayId) { return nullptr; }
    virtual sptr<CutoutInfo> GetCutoutInfo(DisplayId displayId, int32_t width, int32_t height,
                                           Rotation rotation) { return nullptr; }
    virtual DMError HasImmersiveWindow(ScreenId screenId, bool &immersive)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual void NotifyDisplayEvent(DisplayEvent event) {}
    virtual bool SetFreeze(std::vector<DisplayId> displayIds, bool isFreeze) { return false; }
    virtual sptr<ScreenInfo> GetScreenInfoById(ScreenId screenId) { return nullptr; }
    virtual sptr<ScreenGroupInfo> GetScreenGroupInfoById(ScreenId screenId) { return nullptr; }
    virtual DMError GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos) { return DMError::DM_OK; }
    virtual DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
        ScreenId& screenGroupId, const RotationOption& rotationOption = {Rotation::ROTATION_0, false},
        bool forceMirror = false) { return DMError::DM_OK; }

    virtual DMError MakeMirrorForRecord(const std::vector<ScreenId>& mainScreenIds,
        std::vector<ScreenId>& mirrorScreenIds, ScreenId &screenGroupId)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
        DMRect mainScreenRegion, ScreenId& screenGroupId) { return DMError::DM_OK; }
    virtual DMError SetMultiScreenMode(ScreenId mainScreenId, ScreenId secondaryScreenId,
        MultiScreenMode screenMode) { return DMError::DM_OK; }
    virtual DMError SetMultiScreenRelativePosition(MultiScreenPositionOptions mainScreenOptions,
        MultiScreenPositionOptions secondScreenOption) { return DMError::DM_OK; }
    virtual DMError MakeExpand(std::vector<ScreenId> screenId, std::vector<Point> startPoints,
        ScreenId& screenGroupId) { return DMError::DM_OK; }
    virtual DMError StopMirror(const std::vector<ScreenId>& mirrorScreenIds) { return DMError::DM_OK; }
    virtual DMError DisableMirror(bool disableOrNot) { return DMError::DM_ERROR_INVALID_PERMISSION; }
    virtual DMError StopExpand(const std::vector<ScreenId>& expandScreenIds) { return DMError::DM_OK; }
    virtual void RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens) {}
    virtual DMError SetScreenActiveMode(ScreenId screenId, uint32_t modeId) { return DMError::DM_OK; }
    virtual DMError SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio) { return DMError::DM_OK; }
    virtual DMError SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio)
    {
        return DMError::DM_OK;
    }
    virtual DMError SetDefaultDensityDpi(ScreenId screenId, float virtualPixelRatio)
    {
        return DMError::DM_OK;
    }
    virtual DMError SetResolution(ScreenId screenId, uint32_t width, uint32_t height,
        float virtualPixelRatio) { return DMError::DM_OK; }
    virtual DMError GetDensityInCurResolution(ScreenId screenId,
        float& virtualPixelRatio) { return DMError::DM_OK; }
    virtual DMError ResizeVirtualScreen(ScreenId screenId, uint32_t width,
        uint32_t height) { return DMError::DM_OK; }
    virtual DMError AddSurfaceNodeToDisplay(DisplayId displayId,
        std::shared_ptr<class RSSurfaceNode>& surfaceNode, bool onTop = true) { return DMError::DM_OK; }
    virtual DMError RemoveSurfaceNodeFromDisplay(DisplayId displayId,
        std::shared_ptr<class RSSurfaceNode>& surfaceNode) { return DMError::DM_OK; }
    virtual void DumpAllScreensInfo(std::string& dumpInfo) {}
    virtual void DumpSpecialScreenInfo(ScreenId id, std::string& dumpInfo) {}
    virtual DMError SetPrimaryDisplaySystemDpi(float dpi) { return DMError::DM_OK; }
    // Fold Screen
    virtual void SetFoldDisplayMode(const FoldDisplayMode displayMode) {}
    virtual void SetFoldDisplayModeAsync(const FoldDisplayMode displayMode) {}
    virtual DMError SetFoldDisplayModeFromJs(const FoldDisplayMode displayMode,
        std::string reason = "") { return DMError::DM_OK; }

    virtual void SetDisplayScale(ScreenId screenId, float scaleX, float scaleY, float pivotX, float pivotY) {}

    virtual void SetFoldStatusLocked(bool locked) {}
    virtual DMError SetFoldStatusLockedFromJs(bool locked) { return DMError::DM_OK; }
    virtual DMError ForceSetFoldStatusAndLock(FoldStatus targetFoldStatus) { return DMError::DM_OK; }
    virtual DMError RestorePhysicalFoldStatus() { return DMError::DM_OK; }
    virtual void SetFoldStatusExpandAndLocked(bool locked) {}

    virtual FoldDisplayMode GetFoldDisplayMode() { return FoldDisplayMode::UNKNOWN; }
    virtual DMError GetPhysicalScreenIds(std::vector<ScreenId>& screenIds) { return DMError::DM_OK; }

    virtual bool IsFoldable() { return false; }
    virtual bool IsCaptured() { return false; }

    virtual FoldStatus GetFoldStatus() { return FoldStatus::UNKNOWN; }
    virtual SuperFoldStatus GetSuperFoldStatus() { return SuperFoldStatus::UNKNOWN; }
    virtual float GetSuperRotation() { return -1.f; }
    virtual void SetLandscapeLockStatus(bool isLocked) {}
    virtual ExtendScreenConnectStatus GetExtendScreenConnectStatus() { return ExtendScreenConnectStatus::UNKNOWN; }
    virtual void SetForceCloseHdr(ScreenId screenid, bool isForceCloseHdr) {}

    virtual sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion() { return nullptr; }
    virtual DMError GetLiveCreaseRegion(FoldCreaseRegion& region)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual DMError MakeUniqueScreen(const std::vector<ScreenId>& screenIds,
        std::vector<DisplayId>& displayIds, const UniqueScreenRotationOptions& options) { return DMError::DM_OK; }

    virtual void SetClient(const sptr<IScreenSessionManagerClient>& client) {}
    virtual void SwitchUser() {}
    virtual VirtualScreenFlag GetVirtualScreenFlag(ScreenId screenId)
    {
        return VirtualScreenFlag::DEFAULT;
    }
    virtual DMError SetVirtualScreenFlag(ScreenId screenId, VirtualScreenFlag screenFlag)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual ScreenProperty GetScreenProperty(ScreenId screenId) { return ScreenProperty(); }
    virtual std::shared_ptr<RSDisplayNode> GetDisplayNode(ScreenId screenId) { return nullptr; }
    virtual void UpdateScreenRotationProperty(ScreenId screenId, const RRectT<float>& bounds, float rotation,
        ScreenPropertyChangeType screenPropertyChangeType, bool isSwitchUser = false) {}
    virtual void UpdateScreenDirectionInfo(ScreenId screenId, const ScreenDirectionInfo& directionInfo,
        ScreenPropertyChangeType screenPropertyChangeType, const RRect& bounds) {}
    virtual void UpdateAvailableArea(ScreenId screenId, DMRect area) {}
    virtual void UpdateSuperFoldAvailableArea(ScreenId screenId, DMRect bArea, DMRect cArea) {}
    virtual void UpdateSuperFoldExpandAvailableArea(ScreenId screenId, DMRect area) {}
    virtual int32_t SetScreenOffDelayTime(int32_t delay) { return 0; }
    virtual int32_t SetScreenOnDelayTime(int32_t delay) { return 0; }
    virtual void SetCameraStatus(int32_t cameraStatus, int32_t cameraPosition) {}
    virtual uint32_t GetCurvedCompressionArea() { return 0; }
    virtual ScreenProperty GetPhyScreenProperty(ScreenId screenId) { return ScreenProperty(); }
    virtual void NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info) {}
    virtual void SetScreenPrivacyState(bool hasPrivate) {}
    virtual void SetPrivacyStateByDisplayId(std::unordered_map<DisplayId, bool>& privacyBundleDisplayId) {}
    virtual void SetScreenPrivacyWindowList(DisplayId id, std::vector<std::string> privacyWindowList) {}
    virtual DMError GetAvailableArea(DisplayId displayId, DMRect& area) { return DMError::DM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual DMError GetExpandAvailableArea(DisplayId displayId, DMRect& area)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual void NotifyFoldToExpandCompletion(bool foldToExpand) {}
    virtual void NotifyScreenConnectCompletion(ScreenId screenId) {}
    virtual void NotifyAodOpCompletion(AodOP op, int32_t result) {}
    virtual void RecordEventFromScb(std::string description, bool needRecordEvent) {}
    virtual DeviceScreenConfig GetDeviceScreenConfig() { return {}; }
    virtual DMError SetVirtualScreenMaxRefreshRate(ScreenId id, uint32_t refreshRate,
        uint32_t& actualRefreshRate) { return DMError::DM_OK; }
    virtual DMError SetVirtualScreenRefreshRate(ScreenId screenId, uint32_t refreshInterval)
    {
        return DMError::DM_OK;
    }
    virtual DMError ProxyForFreeze(const std::set<int32_t>& pidList, bool isProxy)
    {
        return DMError::DM_OK;
    }
    virtual DMError ResetAllFreezeStatus()
    {
        return DMError::DM_OK;
    }
    virtual void SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList,
        std::vector<uint64_t> surfaceIdList = {}, std::vector<uint8_t> typeBlackList = {}) {}
    virtual void SetVirtualDisplayMuteFlag(ScreenId screenId, bool muteFlag) {}
    virtual void DisablePowerOffRenderControl(ScreenId screenId) {}

    virtual std::vector<DisplayPhysicalResolution> GetAllDisplayPhysicalResolution()
    {
        return std::vector<DisplayPhysicalResolution> {};
    }
    virtual DMError GetDisplayCapability(std::string& capabilitInfo)
    {
        return DMError::DM_OK;
    }
    virtual bool SetVirtualScreenStatus(ScreenId screenId, VirtualScreenStatus screenStatus) { return false; }
    virtual DMError SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid,
        std::vector<uint64_t>& windowIdList)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual std::shared_ptr<Media::PixelMap> GetScreenCapture(const CaptureOption& captureOption,
        DmErrorCode* errorCode = nullptr)
    {
        *errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
        return nullptr;
    }
    virtual sptr<DisplayInfo> GetPrimaryDisplayInfo()
    {
        return nullptr;
    }
    virtual DisplayId GetPrimaryDisplayId()
    {
        return SCREEN_ID_INVALID;
    }
    virtual std::shared_ptr<Media::PixelMap> GetDisplaySnapshotWithOption(const CaptureOption& captureOption,
        DmErrorCode* errorCode = nullptr)
    {
        *errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
        return nullptr;
    }
    virtual std::vector<std::shared_ptr<Media::PixelMap>> GetDisplayHDRSnapshotWithOption(
        const CaptureOption& captureOption, DmErrorCode& errorCode)
    {
        errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
        return { nullptr, nullptr };
    }
    virtual DMError SetScreenSkipProtectedWindow(const std::vector<ScreenId> &screenIds, bool isEnable)
    {
        return DMError::DM_OK;
    }
    virtual ScreenCombination GetScreenCombination(ScreenId screenId) { return ScreenCombination::SCREEN_ALONE; }
    virtual bool GetIsRealScreen(ScreenId screenId) { return false; }
    virtual DMError SetSystemKeyboardStatus(bool isTpKeyboardOn = false)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual void SetDefaultMultiScreenModeWhenSwitchUser() {}
    virtual void NotifyExtendScreenCreateFinish() {}
    virtual void NotifyExtendScreenDestroyFinish() {}
    virtual void NotifyScreenMaskAppear() {}
    virtual bool GetKeyboardState() { return false; }
    virtual uint32_t GetDeviceStatus() { return 0; }
    virtual DMError GetScreenAreaOfDisplayArea(DisplayId displayId, const DMRect& displayArea,
        ScreenId& screenId, DMRect& screenArea) { return DMError::DM_OK; }
    virtual DMError GetBrightnessInfo(DisplayId displayId,
        ScreenBrightnessInfo& brightnessInfo) { return DMError::DM_OK; }
    virtual DMError GetSupportsInput(DisplayId displayId, bool& supportsInput) { return DMError::DM_OK; };
    virtual DMError SetSupportsInput(DisplayId displayId, bool supportsInput) { return DMError::DM_OK; };
    virtual DMError SetVirtualScreenAutoRotation(ScreenId screenId, bool enable) { return DMError::DM_OK; }
    virtual DMError SetScreenPrivacyWindowTagSwitch(ScreenId screenId, const std::vector<std::string>& privacyWindowTag,
        bool enable) { return DMError::DM_OK; }
    virtual bool SynchronizePowerStatus(ScreenPowerState state) { return false; }
    virtual void NotifySwitchUserAnimationFinish() {}
    virtual DMError SyncScreenPropertyChangedToServer(ScreenId screenId, const ScreenProperty& screenProperty)
    {
        return DMError::DM_OK;
    }
    virtual DMError GetRoundedCorner(DisplayId displayId, int& radius) { return DMError::DM_OK; }
    virtual DMError GetBundleName(DisplayId displayId, std::string& bundleName) { return DMError::DM_OK; }
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_SCREEN_SESSION_MANAGER_INTERFACE_H
