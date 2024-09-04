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

#ifndef FOUNDATION_DMSERVER_DISPLAY_MANAGER_INTERFACE_H
#define FOUNDATION_DMSERVER_DISPLAY_MANAGER_INTERFACE_H

#include <iremote_broker.h>
#include <pixel_map.h>
#include <surface.h>
#include <set>

#include "display_cutout_controller.h"
#include "display_info.h"
#include "dm_common.h"
#include "fold_screen_info.h"
#include "screen.h"
#include "screen_info.h"
#include "screen_group_info.h"
#include "zidl/display_manager_agent_interface.h"

namespace OHOS::Rosen {
class IDisplayManager : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.IDisplayManager");

    enum class DisplayManagerMessage : uint32_t {
        TRANS_ID_GET_DEFAULT_DISPLAY_INFO = 0,
        TRANS_ID_GET_DISPLAY_BY_ID,
        TRANS_ID_GET_DISPLAY_BY_SCREEN,
        TRANS_ID_GET_DISPLAY_SNAPSHOT,
        TRANS_ID_REGISTER_DISPLAY_MANAGER_AGENT,
        TRANS_ID_UNREGISTER_DISPLAY_MANAGER_AGENT,
        TRANS_ID_WAKE_UP_BEGIN,
        TRANS_ID_WAKE_UP_END,
        TRANS_ID_SUSPEND_BEGIN,
        TRANS_ID_SUSPEND_END,
        TRANS_ID_SET_SPECIFIED_SCREEN_POWER,
        TRANS_ID_SET_SCREEN_POWER_FOR_ALL,
        TRANS_ID_GET_SCREEN_POWER,
        TRANS_ID_SET_DISPLAY_STATE,
        TRANS_ID_GET_DISPLAY_STATE,
        TRANS_ID_GET_ALL_DISPLAYIDS,
        TRANS_ID_NOTIFY_DISPLAY_EVENT,
        TRANS_ID_SET_FREEZE_EVENT,
        TRANS_ID_SCREEN_BASE = 1000,
        TRANS_ID_CREATE_VIRTUAL_SCREEN = TRANS_ID_SCREEN_BASE,
        TRANS_ID_DESTROY_VIRTUAL_SCREEN,
        TRANS_ID_SET_VIRTUAL_SCREEN_SURFACE,
        TRANS_ID_SET_VIRTUAL_SCREEN_CANVAS_ROTATION,
        TRANS_ID_GET_SCREEN_INFO_BY_ID,
        TRANS_ID_GET_SCREEN_GROUP_INFO_BY_ID,
        TRANS_ID_SET_SCREEN_ACTIVE_MODE,
        TRANS_ID_GET_ALL_SCREEN_INFOS,
        TRANS_ID_SET_ORIENTATION,
        TRANS_ID_SET_VIRTUAL_PIXEL_RATIO,
        TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM,
        TRANS_ID_SET_RESOLUTION,
        TRANS_ID_GET_DENSITY_IN_CURRENT_RESOLUTION,
        TRANS_ID_SCREENGROUP_BASE = 1100,
        TRANS_ID_SCREEN_MAKE_MIRROR = TRANS_ID_SCREENGROUP_BASE,
        TRANS_ID_SCREEN_MAKE_EXPAND,
        TRANS_ID_REMOVE_VIRTUAL_SCREEN_FROM_SCREEN_GROUP,
        TRANS_ID_SCREEN_GAMUT_BASE = 1200,
        TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_GAMUTS = TRANS_ID_SCREEN_GAMUT_BASE,
        TRANS_ID_SCREEN_GET_COLOR_GAMUT,
        TRANS_ID_SCREEN_SET_COLOR_GAMUT,
        TRANS_ID_SCREEN_GET_GAMUT_MAP,
        TRANS_ID_SCREEN_SET_GAMUT_MAP,
        TRANS_ID_SCREEN_SET_COLOR_TRANSFORM,
        TRANS_ID_SCREEN_GET_PIXEL_FORMAT,
        TRANS_ID_SCREEN_SET_PIXEL_FORMAT,
        TRANS_ID_SCREEN_GET_SUPPORTED_HDR_FORMAT,
        TRANS_ID_SCREEN_GET_HDR_FORMAT,
        TRANS_ID_SCREEN_SET_HDR_FORMAT,
        TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_SPACE,
        TRANS_ID_SCREEN_GET_COLOR_SPACE,
        TRANS_ID_SCREEN_SET_COLOR_SPACE,
        TRANS_ID_IS_SCREEN_ROTATION_LOCKED,
        TRANS_ID_SET_SCREEN_ROTATION_LOCKED,
        TRANS_ID_SET_SCREEN_ROTATION_LOCKED_FROM_JS,
        TRANS_ID_HAS_PRIVATE_WINDOW,
        TRANS_ID_GET_CUTOUT_INFO,
        TRANS_ID_HAS_IMMERSIVE_WINDOW,
        TRANS_ID_ADD_SURFACE_NODE,
        TRANS_ID_REMOVE_SURFACE_NODE,
        TRANS_ID_SCREEN_STOP_MIRROR,
        TRANS_ID_SCREEN_STOP_EXPAND,
        TRANS_ID_SCREEN_DISABLE_MIRROR,
        TRANS_ID_SCENE_BOARD_SCREEN_BASE = 2000,
        TRANS_ID_SCENE_BOARD_DUMP_ALL_SCREEN,
        TRANS_ID_SCENE_BOARD_DUMP_SPECIAL_SCREEN,
        TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE,
        TRANS_ID_SET_FOLD_DISPLAY_MODE_FROM_JS,
        TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE,
        TRANS_ID_SCENE_BOARD_IS_FOLDABLE,
        TRANS_ID_SCENE_BOARD_GET_FOLD_STATUS,
        TRANS_ID_SCENE_BOARD_GET_CURRENT_FOLD_CREASE_REGION,
        TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN,
        TRANS_ID_SCENE_BOARD_LOCK_FOLD_DISPLAY_STATUS,
        TRANS_ID_SET_LOCK_FOLD_DISPLAY_STATUS_FROM_JS,
        TRANS_ID_SET_CLIENT = 2500,
        TRANS_ID_GET_SCREEN_PROPERTY,
        TRANS_ID_GET_DISPLAY_NODE,
        TRANS_ID_UPDATE_SCREEN_ROTATION_PROPERTY,
        TRANS_ID_UPDATE_AVAILABLE_AREA,
        TRANS_ID_SET_SCREEN_OFF_DELAY_TIME,
        TRANS_ID_GET_CURVED_SCREEN_COMPRESSION_AREA,
        TRANS_ID_GET_PHY_SCREEN_PROPERTY,
        TRANS_ID_NOTIFY_DISPLAY_CHANGE_INFO,
        TRANS_ID_SET_SCREEN_PRIVACY_STATE,
        TRANS_ID_SET_SCREENID_PRIVACY_STATE,
        TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_LIST,
        TRANS_ID_RESIZE_VIRTUAL_SCREEN,
        TRANS_ID_GET_AVAILABLE_AREA,
        TRANS_ID_NOTIFY_FOLD_TO_EXPAND_COMPLETION,
        TRANS_ID_CONVERT_SCREENID_TO_RSSCREENID,
        TRANS_ID_GET_VIRTUAL_SCREEN_FLAG,
        TRANS_ID_SET_VIRTUAL_SCREEN_FLAG,
        TRANS_ID_SET_VIRTUAL_SCREEN_SCALE_MODE,
        TRANS_ID_GET_DEVICE_SCREEN_CONFIG,
        TRANS_ID_SET_VIRTUAL_SCREEN_REFRESH_RATE,
        TRANS_ID_DEVICE_IS_CAPTURE,
        TRANS_ID_GET_SNAPSHOT_BY_PICKER,
        TRANS_ID_SWITCH_USER,
        TRANS_ID_SET_VIRTUAL_SCREEN_BLACK_LIST,
        TRANS_ID_DISABLE_POWEROFF_RENDER_CONTROL,
        TRANS_ID_PROXY_FOR_FREEZE,
        TRANS_ID_RESET_ALL_FREEZE_STATUS,
        TRANS_ID_NOTIFY_DISPLAY_HOOK_INFO,
        TRANS_ID_GET_ALL_PHYSICAL_DISPLAY_RESOLUTION,
        TRANS_ID_SET_VIRTUAL_SCREEN_SECURITY_EXEMPTION,
    };

    virtual sptr<DisplayInfo> GetDefaultDisplayInfo() = 0;
    virtual sptr<DisplayInfo> GetDisplayInfoById(DisplayId displayId) = 0;
    virtual sptr<DisplayInfo> GetDisplayInfoByScreen(ScreenId screenId) = 0;
    virtual DMError HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow) = 0;
    virtual bool ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId) { return false; };
    virtual void UpdateDisplayHookInfo(int32_t uid, bool enable, const DMHookInfo& hookInfo) {};
    virtual ScreenId CreateVirtualScreen(VirtualScreenOption option,
        const sptr<IRemoteObject>& displayManagerAgent) = 0;
    virtual DMError DestroyVirtualScreen(ScreenId screenId) = 0;
    virtual DMError SetVirtualScreenSurface(ScreenId screenId, sptr<IBufferProducer> surface) = 0;
    virtual DMError SetVirtualMirrorScreenCanvasRotation(ScreenId screenId, bool rotate) { return DMError::DM_OK; }
    virtual DMError SetVirtualMirrorScreenScaleMode(ScreenId screenId, ScreenScaleMode scaleMode)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual DMError SetOrientation(ScreenId screenId, Orientation orientation) = 0;
    virtual std::shared_ptr<Media::PixelMap> GetDisplaySnapshot(DisplayId displayId,
        DmErrorCode* errorCode = nullptr) = 0;
    virtual std::shared_ptr<Media::PixelMap> GetSnapshotByPicker(Media::Rect &rect, DmErrorCode* errorCode = nullptr)
    {
        *errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
        return nullptr;
    }
    virtual DMError SetScreenRotationLocked(bool isLocked) = 0;
    virtual DMError SetScreenRotationLockedFromJs(bool isLocked) = 0;
    virtual DMError IsScreenRotationLocked(bool& isLocked) = 0;

    // colorspace, gamut
    virtual DMError GetScreenSupportedColorGamuts(ScreenId screenId, std::vector<ScreenColorGamut>& colorGamuts) = 0;
    virtual DMError GetScreenColorGamut(ScreenId screenId, ScreenColorGamut& colorGamut) = 0;
    virtual DMError SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx) = 0;
    virtual DMError GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap) = 0;
    virtual DMError SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap) = 0;
    virtual DMError SetScreenColorTransform(ScreenId screenId) = 0;

    virtual DMError GetPixelFormat(ScreenId screenId, GraphicPixelFormat& pixelFormat)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual DMError SetPixelFormat(ScreenId screenId, GraphicPixelFormat pixelFormat)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual DMError GetSupportedHDRFormats(ScreenId screenId,
        std::vector<ScreenHDRFormat>& hdrFormats)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual DMError GetScreenHDRFormat(ScreenId screenId, ScreenHDRFormat& hdrFormat)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual DMError SetScreenHDRFormat(ScreenId screenId, int32_t modeIdx)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual DMError GetSupportedColorSpaces(ScreenId screenId,
        std::vector<GraphicCM_ColorSpaceType>& colorSpaces)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual DMError GetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType& colorSpace)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual DMError SetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType colorSpace)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual DMError RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type) = 0;
    virtual DMError UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type) = 0;
    virtual bool WakeUpBegin(PowerStateChangeReason reason) = 0;
    virtual bool WakeUpEnd() = 0;
    virtual bool SuspendBegin(PowerStateChangeReason reason) = 0;
    virtual bool SuspendEnd() = 0;
    virtual bool SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason) = 0;
    virtual bool SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason) = 0;
    virtual ScreenPowerState GetScreenPower(ScreenId dmsScreenId) = 0;
    virtual bool SetDisplayState(DisplayState state) = 0;
    virtual DisplayState GetDisplayState(DisplayId displayId) = 0;
    virtual std::vector<DisplayId> GetAllDisplayIds() = 0;
    virtual sptr<CutoutInfo> GetCutoutInfo(DisplayId displayId) = 0;
    virtual void NotifyDisplayEvent(DisplayEvent event) = 0;
    virtual bool SetFreeze(std::vector<DisplayId> displayIds, bool isFreeze) = 0;
    virtual sptr<ScreenInfo> GetScreenInfoById(ScreenId screenId) = 0;
    virtual sptr<ScreenGroupInfo> GetScreenGroupInfoById(ScreenId screenId) = 0;
    virtual DMError GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos) = 0;
    virtual DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
        ScreenId& screenGroupId) = 0;
    virtual DMError MakeExpand(std::vector<ScreenId> screenId, std::vector<Point> startPoints,
        ScreenId& screenGroupId) = 0;
    virtual DMError StopMirror(const std::vector<ScreenId>& mirrorScreenIds) = 0;
    virtual DMError StopExpand(const std::vector<ScreenId>& expandScreenIds) = 0;
    virtual DMError DisableMirror(bool disableOrNot) { return DMError::DM_ERROR_INVALID_PERMISSION; }
    virtual void RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens) = 0;
    virtual DMError SetScreenActiveMode(ScreenId screenId, uint32_t modeId) = 0;
    virtual DMError SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio) = 0;
    virtual DMError SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual DMError SetResolution(ScreenId screenId, uint32_t width, uint32_t height, float virtualPixelRatio) = 0;
    virtual DMError GetDensityInCurResolution(ScreenId screenId, float& virtualPixelRatio) = 0;
    virtual DMError ResizeVirtualScreen(ScreenId screenId, uint32_t width, uint32_t height) { return DMError::DM_OK; }
    virtual DMError AddSurfaceNodeToDisplay(DisplayId displayId,
        std::shared_ptr<class RSSurfaceNode>& surfaceNode, bool onTop = true) = 0;
    virtual DMError RemoveSurfaceNodeFromDisplay(DisplayId displayId,
        std::shared_ptr<class RSSurfaceNode>& surfaceNode) = 0;
    virtual DMError GetAvailableArea(DisplayId displayId, DMRect& area) { return DMError::DM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual bool IsFoldable() { return false; }
    virtual bool IsCaptured() { return false; }

    virtual FoldStatus GetFoldStatus() { return FoldStatus::UNKNOWN; }

    virtual FoldDisplayMode GetFoldDisplayMode() { return FoldDisplayMode::UNKNOWN; }

    virtual void SetFoldDisplayMode(const FoldDisplayMode) {}

    virtual DMError SetFoldDisplayModeFromJs(const FoldDisplayMode) { return DMError::DM_OK; }

    virtual void SetFoldStatusLocked(bool locked) {}

    virtual DMError SetFoldStatusLockedFromJs(bool locked) { return DMError::DM_OK; }

    virtual sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion() { return nullptr; }

    virtual DMError HasImmersiveWindow(bool& immersive) { return DMError::DM_ERROR_DEVICE_NOT_SUPPORT; }

    // unique screen
    virtual DMError MakeUniqueScreen(const std::vector<ScreenId>& screenIds) { return DMError::DM_OK; }

    virtual VirtualScreenFlag GetVirtualScreenFlag(ScreenId screenId)
    {
        return VirtualScreenFlag::DEFAULT;
    }
    virtual DMError SetVirtualScreenFlag(ScreenId screenId, VirtualScreenFlag screenFlag)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
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
    virtual void SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList) {}
    virtual void DisablePowerOffRenderControl(ScreenId screenId) {}

    virtual std::vector<DisplayPhysicalResolution> GetAllDisplayPhysicalResolution()
    {
        return std::vector<DisplayPhysicalResolution> {};
    }
    virtual DMError SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid,
        std::vector<uint64_t>& windowIdList)
    {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
};
} // namespace OHOS::Rosen

#endif // FOUNDATION_DMSERVER_DISPLAY_MANAGER_INTERFACE_H