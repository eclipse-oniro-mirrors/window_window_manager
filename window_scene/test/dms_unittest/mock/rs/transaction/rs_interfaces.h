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

#ifndef RENDER_SERVICE_CLIENT_CORE_TRANSACTION_RS_INTERFACES_H
#define RENDER_SERVICE_CLIENT_CORE_TRANSACTION_RS_INTERFACES_H

#include <memory>
#include <mutex>

#include "rs_screen_capability.h"
#include "ui/rs_display_node.h"
#include "ui/rs_surface_node.h"
#include "pixel_map.h"
#include "transaction/rs_render_service_client.h"
#include "screen_manager/rs_screen_mode_info.h"
#include "common/rs_event_def.h"

namespace OHOS {
namespace Rosen {
constexpr float EPSILON = std::numeric_limits<float>::epsilon();
struct BrightnessInfo {
    float currentHeadroom = 1.0f;
    float maxHeadroom = 1.0f;
    float sdrNits = 500.0f;

    bool operator==(const BrightnessInfo& other) const
    {
        return this == &other || (ROSEN_EQ(currentHeadroom, other.currentHeadroom) &&
                                  ROSEN_EQ(maxHeadroom, other.maxHeadroom) && ROSEN_EQ(sdrNits, other.sdrNits));
    }

    bool operator!=(const BrightnessInfo& other) const
    {
        return !(*this == other);
    }
};

struct EventInfo {
    std::string eventName;    // unique ID assigned by HGM, e.g."VOTER_THERMAL"
    bool eventStatus;         // true-enter, false-exit
    uint32_t minRefreshRate;  // the desired min refresh rate, e.g.60
    uint32_t maxRefreshRate;  // the desired max refresh rate, e.g.120
    std::string description;  // the extend description for eventName，e.g."SCENE_APP_START_ANIMATION"
};

using ScreenChangeCallback = std::function<void(ScreenId, ScreenEvent, ScreenChangeReason, sptr<IRemoteObject>)>;
using ScreenSwitchingNotifyCallback = std::function<void(bool)>;
using BrightnessInfoChangeCallback = std::function<void(ScreenId, BrightnessInfo)>;
using HgmRefreshRateUpdateCallback = std::function<void(int32_t)>;
using FirstFrameCommitCallback = std::function<void(uint64_t, int64_t)>;
using ScreenSupportedHDRFormatsCallback = std::function<void(ScreenId,
    std::vector<ScreenHDRFormat>& specialHdrFormats)>;

class RSInterfaces {
public:
    static RSInterfaces& GetInstance()
    {
        static RSInterfaces instance;
        return instance;
    }

    ScreenId GetDefaultScreenId();
    ScreenId CreateVirtualScreen(const std::string& name, uint32_t width, uint32_t height, sptr<Surface> surface,
                                 ScreenId associatedScreenId = 0, int flags = 0, std::vector<NodeId> whiteList = {});
    int32_t SetVirtualScreenTypeBlackList(ScreenId id, std::vector<NodeType>& typeBlackListVector);
    int32_t SetVirtualScreenSecurityExemptionList(ScreenId id, const std::vector<NodeId>& securityExemptionList);
    int32_t SetScreenSecurityMask(ScreenId id, std::shared_ptr<Media::PixelMap> securityMask);
    int32_t SetMirrorScreenVisibleRect(ScreenId id, const Rect& mainScreenRect, bool supportRotation = false);
    int32_t SetCastScreenEnableSkipWindow(ScreenId id, bool enable);
    int32_t SetVirtualScreenSurface(ScreenId id, sptr<Surface> surface);
    void RemoveVirtualScreen(ScreenId id);
    int32_t SetScreenChangeCallback(const ScreenChangeCallback& callback);
    int32_t SetScreenSwitchingNotifyCallback(const ScreenSwitchingNotifyCallback& callback);
    int32_t SetBrightnessInfoChangeCallback(const BrightnessInfoChangeCallback& callback);
    int32_t GetBrightnessInfo(ScreenId screenId, BrightnessInfo& brightnessInfo);
    bool TakeSurfaceCapture(std::shared_ptr<RSDisplayNode> node, std::shared_ptr<SurfaceCaptureCallback> callback,
                            RSSurfaceCaptureConfig captureConfig = {});
    bool FreezeScreen(std::shared_ptr<RSDisplayNode> node, bool isFreeze, bool needSync = false);
    uint32_t SetScreenActiveMode(ScreenId id, uint32_t modeId);
    int32_t SetRogScreenResolution(ScreenId id, uint32_t width, uint32_t height);
    int32_t SetPhysicalScreenResolution(ScreenId id, uint32_t width, uint32_t height);
    bool SetVirtualMirrorScreenCanvasRotation(ScreenId id, bool canvasRotation);
    int32_t SetVirtualScreenAutoRotation(ScreenId id, bool isAutoRotation);
    bool SetVirtualMirrorScreenScaleMode(ScreenId id, ScreenScaleMode scaleMode);
    void MarkPowerOffNeedProcessOneFrame();
    void DisablePowerOffRenderControl(ScreenId id);
    void SetScreenPowerStatus(ScreenId id, ScreenPowerStatus status);
    int32_t SetDualScreenState(ScreenId id, DualScreenStatus status);
    RSScreenModeInfo GetScreenActiveMode(ScreenId id);
    std::vector<RSScreenModeInfo> GetScreenSupportedModes(ScreenId id);
    RSScreenCapability GetScreenCapability(ScreenId id);
    ScreenPowerStatus GetScreenPowerStatus(ScreenId id);
    PanelPowerStatus GetPanelPowerStatus(ScreenId id);
    int32_t GetScreenBacklight(ScreenId id);
    void SetScreenBacklight(ScreenId id, uint32_t level);
    int32_t GetScreenSupportedColorGamuts(ScreenId id, std::vector<ScreenColorGamut>& mode);
    int32_t GetScreenColorGamut(ScreenId id, ScreenColorGamut& mode);
    int32_t SetScreenColorGamut(ScreenId id, int32_t modeIdx);
    int32_t SetScreenGamutMap(ScreenId id, ScreenGamutMap mode);
    int32_t SetScreenCorrection(ScreenId id, ScreenRotation screenRotation);
    int32_t GetScreenGamutMap(ScreenId id, ScreenGamutMap& mode);
    int32_t GetPixelFormat(ScreenId id, GraphicPixelFormat& pixelFormat);
    int32_t SetPixelFormat(ScreenId id, GraphicPixelFormat pixelFormat);
    int32_t GetScreenSupportedHDRFormats(ScreenId id, std::vector<ScreenHDRFormat>& hdrFormats,
        const ScreenSupportedHDRFormatsCallback& callback = nullptr);
    int32_t GetScreenHDRFormat(ScreenId id, ScreenHDRFormat& hdrFormat);
    int32_t GetScreenHDRStatus(ScreenId id, HdrStatus& hdrStatus);
    int32_t SetScreenHDRFormat(ScreenId id, int32_t modeIdx);
    int32_t GetScreenSupportedColorSpaces(ScreenId id, std::vector<GraphicCM_ColorSpaceType>& colorSpaces);
    int32_t GetScreenColorSpace(ScreenId id, GraphicCM_ColorSpaceType& colorSpace);
    int32_t SetScreenColorSpace(ScreenId id, GraphicCM_ColorSpaceType colorSpace);
    int32_t GetScreenType(ScreenId id, RSScreenType& screenType);
    int32_t GetDisplayIdentificationData(ScreenId id, uint8_t& outPort, std::vector<uint8_t>& edidData);
    int32_t SetScreenSkipFrameInterval(ScreenId id, uint32_t skipFrameInterval);
    int32_t SetVirtualScreenRefreshRate(ScreenId id, uint32_t maxRefreshRate, uint32_t& actualRefreshRate);
    uint32_t SetScreenActiveRect(ScreenId id, const Rect& activeRect);
    void SetScreenOffset(ScreenId id, int32_t offsetX, int32_t offsetY);
    void SetScreenFrameGravity(ScreenId id, int32_t gravity);
    int32_t RegisterHgmRefreshRateUpdateCallback(const HgmRefreshRateUpdateCallback& callback);
    int32_t RegisterFirstFrameCommitCallback(const FirstFrameCommitCallback& callback);
    int32_t ResizeVirtualScreen(ScreenId id, uint32_t width, uint32_t height);
    void NotifyRefreshRateEvent(const EventInfo& eventInfo);
    void EnableCacheForRotation();
    void DisableCacheForRotation();
    void SetTpFeatureConfig(int32_t feature, const char* config,
                            TpFeatureConfigType tpFeatureConfigType = TpFeatureConfigType::DEFAULT_TP_FEATURE);
    bool SetVirtualScreenStatus(ScreenId id, VirtualScreenStatus screenStatus);
    void ForceRefreshOneFrameWithNextVSync();
    void NotifyScreenSwitched();
    bool TakeSurfaceCaptureWithAllWindows(std::shared_ptr<RSDisplayNode> node,
                                          std::shared_ptr<SurfaceCaptureCallback> callback,
                                          RSSurfaceCaptureConfig captureConfig, bool checkDrmAndSurfaceLock);
    int32_t AddVirtualScreenWhiteList(ScreenId id, const std::vector<NodeId>& whiteList);
    int32_t RemoveVirtualScreenWhiteList(ScreenId id, const std::vector<NodeId>& whiteList);
    int32_t SetLogicalCameraRotationCorrection(ScreenId id, ScreenRotation screenRotation);
    ScreenId GetActiveScreenId();
    int32_t RegisterExposedEventCallback(const RSExposedEventType type, const RSExposedEventCallback& callback);
    int32_t UnRegisterExposedEventCallback(const RSExposedEventType type);
};
}  // namespace Rosen
}  // namespace OHOS

#endif  // RENDER_SERVICE_CLIENT_CORE_TRANSACTION_RS_INTERFACES_H
