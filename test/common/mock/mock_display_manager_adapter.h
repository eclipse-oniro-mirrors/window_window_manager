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

#ifndef FRAMEWORKS_WM_TEST_UT_MOCK_DISPLAY_MANAGER_ADAPTER_H
#define FRAMEWORKS_WM_TEST_UT_MOCK_DISPLAY_MANAGER_ADAPTER_H
#include <gmock/gmock.h>


#include "display_manager.h"
#include "display_manager_adapter.h"

namespace OHOS {
namespace Rosen {
class MockDisplayManagerAdapter : public DisplayManagerAdapter {
public:
    MOCK_METHOD0(Clear, void());
    MOCK_METHOD2(RegisterDisplayManagerAgent, DMError(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type));
    MOCK_METHOD2(UnregisterDisplayManagerAgent, DMError(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type));
    MOCK_METHOD1(GetDefaultDisplayInfo, sptr<DisplayInfo>(int32_t userId));
    MOCK_METHOD1(GetDisplayInfoByScreenId, sptr<DisplayInfo>(ScreenId screenId));
    MOCK_METHOD4(GetDisplaySnapshot, std::shared_ptr<Media::PixelMap>(DisplayId displayId, DmErrorCode* errorCode,
        bool isUseDma, bool isCaptureFullOfScreen));

    MOCK_METHOD1(WakeUpBegin, bool(PowerStateChangeReason reason));
    MOCK_METHOD0(WakeUpEnd, bool());
    MOCK_METHOD1(SuspendBegin, bool(PowerStateChangeReason reason));
    MOCK_METHOD0(SuspendEnd, bool());
    MOCK_METHOD1(SetDisplayState, bool(DisplayState state));
    MOCK_METHOD1(GetDisplayState, DisplayState(DisplayId displayId));
    MOCK_METHOD1(NotifyDisplayEvent, void(DisplayEvent event));
    MOCK_METHOD1(GetDisplayInfo, sptr<DisplayInfo>(DisplayId displayId));
    MOCK_METHOD4(GetCutoutInfo, sptr<CutoutInfo>(DisplayId displayId, int32_t width, int32_t height,
        Rotation rotation));
    MOCK_METHOD2(GetAvailableArea, DMError(DisplayId displayId, DMRect& area));
    MOCK_METHOD2(HasImmersiveWindow, DMError(ScreenId screenId, bool& immersive));
    MOCK_METHOD4(GetDisplayHDRSnapshot, std::vector<std::shared_ptr<Media::PixelMap>>(DisplayId displayId,
        DmErrorCode& errorCode, bool isUseDma, bool isCaptureFullOfScreen));
    MOCK_METHOD2(GetDisplayHDRSnapshotWithOption, std::vector<std::shared_ptr<Media::PixelMap>>(
        const CaptureOption& captureOption, DmErrorCode& errorCode));
    MOCK_METHOD1(GetAllDisplayIds, std::vector<DisplayId>(int32_t userId));
};

class MockScreenManagerAdapter : public ScreenManagerAdapter {
public:
    MOCK_METHOD0(Clear, void());
    MOCK_METHOD2(RegisterDisplayManagerAgent, DMError(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type));
    MOCK_METHOD2(UnregisterDisplayManagerAgent, DMError(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type));
    MOCK_METHOD2(CreateVirtualScreen, ScreenId(VirtualScreenOption option,
        const sptr<IDisplayManagerAgent>& displayManagerAgent));
    MOCK_METHOD2(DestroyVirtualScreen, DMError(ScreenId screenId, bool isCallingByThirdParty));
    MOCK_METHOD2(SetVirtualScreenSurface, DMError(ScreenId screenId, sptr<Surface> surface));
    MOCK_METHOD1(GetScreenGroupInfoById, sptr<ScreenGroupInfo>(ScreenId screenId));
    MOCK_METHOD1(GetAllScreenInfos, DMError(std::vector<sptr<ScreenInfo>>& screenInfos));
    MOCK_METHOD4(MakeMirror, DMError(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId,
        ScreenId& screenGroupId, const RotationOption& rotationOption));
    MOCK_METHOD3(MakeExpand, DMError(std::vector<ScreenId> screenId, std::vector<Point> startPoint,
        ScreenId& screenGroupId));
    MOCK_METHOD2(SetScreenActiveMode, DMError(ScreenId screenId, uint32_t modeId));
    MOCK_METHOD1(GetScreenInfo, sptr<ScreenInfo>(ScreenId screenId));
    MOCK_METHOD2(SetScreenPowerForAll, bool(ScreenPowerState state, PowerStateChangeReason reason));
    MOCK_METHOD1(GetScreenPower, ScreenPowerState(ScreenId dmsScreenId));

    MOCK_METHOD2(GetScreenSupportedColorGamuts, DMError(ScreenId screenId, std::vector<ScreenColorGamut>& colorGamuts));
    MOCK_METHOD2(GetScreenColorGamut, DMError(ScreenId screenId, ScreenColorGamut& colorGamut));
    MOCK_METHOD2(SetScreenColorGamut, DMError(ScreenId screenId, int32_t colorGamutIdx));
    MOCK_METHOD2(GetScreenGamutMap, DMError(ScreenId screenId, ScreenGamutMap& gamutMap));
    MOCK_METHOD2(SetScreenGamutMap, DMError(ScreenId screenId, ScreenGamutMap gamutMap));
    MOCK_METHOD1(SetScreenColorTransform, DMError(ScreenId screenId));

    MOCK_METHOD1(RemoveVirtualScreenFromGroup, void(std::vector<ScreenId> screens));
    MOCK_METHOD1(SetScreenRotationLocked, DMError(bool isLocked));
    MOCK_METHOD1(IsScreenRotationLocked, DMError(bool& isLocked));
    MOCK_METHOD4(SetResolution, DMError(ScreenId screenId, uint32_t width, uint32_t height, float virtualPixelRatio));
    MOCK_METHOD2(GetPixelFormat, DMError(ScreenId screenId, GraphicPixelFormat& pixelFormat));
    MOCK_METHOD2(SetPixelFormat, DMError(ScreenId screenId, GraphicPixelFormat pixelFormat));
    MOCK_METHOD2(GetDensityInCurResolution, DMError(ScreenId screenId, float& virtualPixelRatio));
    MOCK_METHOD2(GetSupportedHDRFormats, DMError(ScreenId screenId, std::vector<ScreenHDRFormat>& hdrFormats));
    MOCK_METHOD2(GetSupportedColorSpaces, DMError(ScreenId screenId,
        std::vector<GraphicCM_ColorSpaceType>& colorSpaces));
    MOCK_METHOD3(SetOrientation, DMError(ScreenId screenId, Orientation orientation, bool isFromNapi));
    MOCK_METHOD2(GetScreenHDRFormat, DMError(ScreenId screenId, ScreenHDRFormat& hdrFormat));
    MOCK_METHOD2(SetScreenHDRFormat, DMError(ScreenId screenId, int32_t modeIdx));
    MOCK_METHOD2(GetScreenColorSpace, DMError(ScreenId screenId, GraphicCM_ColorSpaceType& colorSpace));
    MOCK_METHOD2(SetScreenColorSpace, DMError(ScreenId screenId, GraphicCM_ColorSpaceType colorSpace));
};
} // namespace Rosen
} // namespace OHOS

#endif // FRAMEWORKS_WM_TEST_UT_MOCK_DISPLAY_MANAGER_ADAPTER_H
