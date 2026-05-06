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

#include "rs_interfaces.h"
#include "rs_mock_impl.h"

namespace OHOS {
namespace Rosen {

ScreenId RSInterfaces::GetDefaultScreenId()
{
    return 0;
}

ScreenId RSInterfaces::CreateVirtualScreen(const std::string& name, uint32_t width, uint32_t height,
                                           sptr<Surface> surface, ScreenId associatedScreenId, int flags,
                                           std::vector<NodeId> whiteList)
{
    return 0;
}

int32_t RSInterfaces::SetVirtualScreenTypeBlackList(ScreenId id, std::vector<NodeType>& typeBlackListVector)
{
    return 0;
}
int32_t RSInterfaces::SetVirtualScreenSecurityExemptionList(ScreenId id,
                                                            const std::vector<NodeId>& securityExemptionList)
{
    return 0;
}
int32_t RSInterfaces::SetScreenSecurityMask(ScreenId id, std::shared_ptr<Media::PixelMap> securityMask)
{
    return 0;
}
int32_t RSInterfaces::SetMirrorScreenVisibleRect(ScreenId id, const Rect& mainScreenRect, bool supportRotation)
{
    return 0;
}
int32_t RSInterfaces::SetCastScreenEnableSkipWindow(ScreenId id, bool enable)
{
    return 0;
}
int32_t RSInterfaces::SetVirtualScreenSurface(ScreenId id, sptr<Surface> surface)
{
    return 0;
}
void RSInterfaces::RemoveVirtualScreen(ScreenId id) {}
int32_t RSInterfaces::SetScreenChangeCallback(const ScreenChangeCallback& callback)
{
    return RsMockImpl::GetInstance().SetScreenChangeCallback(callback);
}
int32_t RSInterfaces::SetScreenSwitchingNotifyCallback(const ScreenSwitchingNotifyCallback& callback)
{
    return 0;
}
int32_t RSInterfaces::SetBrightnessInfoChangeCallback(const BrightnessInfoChangeCallback& callback)
{
    return 0;
}
int32_t RSInterfaces::GetBrightnessInfo(ScreenId screenId, BrightnessInfo& brightnessInfo)
{
    return 0;
}
bool RSInterfaces::TakeSurfaceCapture(std::shared_ptr<RSDisplayNode> node,
                                      std::shared_ptr<SurfaceCaptureCallback> callback,
                                      RSSurfaceCaptureConfig captureConfig)
{
    return true;
}
bool RSInterfaces::FreezeScreen(std::shared_ptr<RSDisplayNode> node, bool isFreeze, bool needSync)
{
    return true;
}
uint32_t RSInterfaces::SetScreenActiveMode(ScreenId id, uint32_t modeId)
{
    return 0;
}
int32_t RSInterfaces::SetRogScreenResolution(ScreenId id, uint32_t width, uint32_t height)
{
    return 0;
}
int32_t RSInterfaces::SetPhysicalScreenResolution(ScreenId id, uint32_t width, uint32_t height)
{
    return 0;
}
bool RSInterfaces::SetVirtualMirrorScreenCanvasRotation(ScreenId id, bool canvasRotation)
{
    return true;
}
int32_t RSInterfaces::SetVirtualScreenAutoRotation(ScreenId id, bool isAutoRotation)
{
    return 0;
}
bool RSInterfaces::SetVirtualMirrorScreenScaleMode(ScreenId id, ScreenScaleMode scaleMode)
{
    return true;
}
void RSInterfaces::MarkPowerOffNeedProcessOneFrame() {}
void RSInterfaces::DisablePowerOffRenderControl(ScreenId id) {}
void RSInterfaces::SetScreenPowerStatus(ScreenId id, ScreenPowerStatus status) {}
int32_t RSInterfaces::SetDualScreenState(ScreenId id, DualScreenStatus status)
{
    return 0;
}
RSScreenModeInfo RSInterfaces::GetScreenActiveMode(ScreenId id)
{
    return RsMockImpl::GetInstance().GetScreenActiveMode(id);
}
std::vector<RSScreenModeInfo> RSInterfaces::GetScreenSupportedModes(ScreenId id)
{
    return RsMockImpl::GetInstance().GetScreenSupportedModes(id);
}

RSScreenCapability RSInterfaces::GetScreenCapability(ScreenId id)
{
    return RSScreenCapability();
}
ScreenPowerStatus RSInterfaces::GetScreenPowerStatus(ScreenId id)
{
    return ScreenPowerStatus::POWER_STATUS_ON;
}
PanelPowerStatus RSInterfaces::GetPanelPowerStatus(ScreenId id)
{
    return PanelPowerStatus::PANEL_POWER_STATUS_ON;
}
int32_t RSInterfaces::GetScreenBacklight(ScreenId id)
{
    return 0;
}

void RSInterfaces::SetScreenBacklight(ScreenId id, uint32_t level) {}
int32_t RSInterfaces::GetScreenSupportedColorGamuts(ScreenId id, std::vector<ScreenColorGamut>& mode)
{
    return 0;
}
int32_t RSInterfaces::GetScreenColorGamut(ScreenId id, ScreenColorGamut& mode)
{
    return 0;
}
int32_t RSInterfaces::SetScreenColorGamut(ScreenId id, int32_t modeIdx)
{
    return 0;
}
int32_t RSInterfaces::SetScreenGamutMap(ScreenId id, ScreenGamutMap mode)
{
    return 0;
}

int32_t RSInterfaces::SetScreenCorrection(ScreenId id, ScreenRotation screenRotation)
{
    return RsMockImpl::GetInstance().SetScreenCorrection(id, screenRotation);
}

int32_t RSInterfaces::GetScreenGamutMap(ScreenId id, ScreenGamutMap& mode)
{
    return 0;
}
int32_t RSInterfaces::GetPixelFormat(ScreenId id, GraphicPixelFormat& pixelFormat)
{
    return 0;
}
int32_t RSInterfaces::SetPixelFormat(ScreenId id, GraphicPixelFormat pixelFormat)
{
    return 0;
}
int32_t RSInterfaces::GetScreenSupportedHDRFormats(ScreenId id, std::vector<ScreenHDRFormat>& hdrFormats,
    const ScreenSupportedHDRFormatsCallback& callback)
{
    return RsMockImpl::GetInstance().GetScreenSupportedHDRFormats(id, hdrFormats);
}
int32_t RSInterfaces::GetScreenHDRFormat(ScreenId id, ScreenHDRFormat& hdrFormat)
{
    return 0;
}
int32_t RSInterfaces::GetScreenHDRStatus(ScreenId id, HdrStatus& hdrStatus)
{
    return 0;
}
int32_t RSInterfaces::SetScreenHDRFormat(ScreenId id, int32_t modeIdx)
{
    return 0;
}
int32_t RSInterfaces::GetScreenSupportedColorSpaces(ScreenId id, std::vector<GraphicCM_ColorSpaceType>& colorSpaces)
{
    return RsMockImpl::GetInstance().GetScreenSupportedColorSpaces(id, colorSpaces);
}
int32_t RSInterfaces::GetScreenColorSpace(ScreenId id, GraphicCM_ColorSpaceType& colorSpace)
{
    return 0;
}
int32_t RSInterfaces::SetScreenColorSpace(ScreenId id, GraphicCM_ColorSpaceType colorSpace)
{
    return 0;
}
int32_t RSInterfaces::GetScreenType(ScreenId id, RSScreenType& screenType)
{
    return 0;
}
int32_t RSInterfaces::GetDisplayIdentificationData(ScreenId id, uint8_t& outPort, std::vector<uint8_t>& edidData)
{
    return 0;
}
int32_t RSInterfaces::SetScreenSkipFrameInterval(ScreenId id, uint32_t skipFrameInterval)
{
    return 0;
}
int32_t RSInterfaces::SetVirtualScreenRefreshRate(ScreenId id, uint32_t maxRefreshRate, uint32_t& actualRefreshRate)
{
    return 0;
}
uint32_t RSInterfaces::SetScreenActiveRect(ScreenId id, const Rect& activeRect)
{
    return 0;
}
void RSInterfaces::SetScreenOffset(ScreenId id, int32_t offsetX, int32_t offsetY) {}
void RSInterfaces::SetScreenFrameGravity(ScreenId id, int32_t gravity) {}
int32_t RSInterfaces::RegisterHgmRefreshRateUpdateCallback(const HgmRefreshRateUpdateCallback& callback)
{
    return RsMockImpl::GetInstance().RegisterHgmRefreshRateUpdateCallback(callback);
}
int32_t RSInterfaces::RegisterFirstFrameCommitCallback(const FirstFrameCommitCallback& callback)
{
    return 0;
}
int32_t RSInterfaces::ResizeVirtualScreen(ScreenId id, uint32_t width, uint32_t height)
{
    return 0;
}
void RSInterfaces::NotifyRefreshRateEvent(const EventInfo& eventInfo) {}
void RSInterfaces::EnableCacheForRotation() {}
void RSInterfaces::DisableCacheForRotation() {}
void RSInterfaces::SetTpFeatureConfig(int32_t feature, const char* config, TpFeatureConfigType tpFeatureConfigType) {}
bool RSInterfaces::SetVirtualScreenStatus(ScreenId id, VirtualScreenStatus screenStatus)
{
    return true;
}
void RSInterfaces::ForceRefreshOneFrameWithNextVSync() {}
void RSInterfaces::NotifyScreenSwitched() {}
bool RSInterfaces::TakeSurfaceCaptureWithAllWindows(std::shared_ptr<RSDisplayNode> node,
                                                    std::shared_ptr<SurfaceCaptureCallback> callback,
                                                    RSSurfaceCaptureConfig captureConfig, bool checkDrmAndSurfaceLock)
{
    return true;
}

int32_t RSInterfaces::AddVirtualScreenWhiteList(ScreenId id, const std::vector<NodeId>& whiteList)
{
    return 0;
}

int32_t RSInterfaces::RemoveVirtualScreenWhiteList(ScreenId id, const std::vector<NodeId>& whiteList)
{
    return 0;
}

int32_t RSInterfaces::SetLogicalCameraRotationCorrection(ScreenId id, ScreenRotation screenRotation)
{
    return 0;
}

ScreenId RSInterfaces::GetActiveScreenId()
{
    return 0;
}

int32_t RSInterfaces::RegisterExposedEventCallback(
    const RSExposedEventType type, const RSExposedEventCallback& callback)
{
    return 0;
}

int32_t RSInterfaces::UnRegisterExposedEventCallback(const RSExposedEventType type)
{
    return 0;
}
}  // namespace Rosen
}  // namespace OHOS