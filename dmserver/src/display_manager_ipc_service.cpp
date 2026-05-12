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

#include "display_manager_ipc_service.h"
#include "scene_board_judgement.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(DisplayManagerIpcService)
const bool REGISTER_RESULT = SceneBoardJudgement::IsSceneBoardEnabled() ? false :
    SystemAbility::MakeAndRegisterAbility(&DisplayManagerIpcService::GetInstance());

DisplayManagerIpcService::DisplayManagerIpcService() : SystemAbility(DISPLAY_MANAGER_SERVICE_SA_ID, true),
    displayManagerService_(DisplayManagerService::GetInstance())
{
}

void DisplayManagerIpcService::OnStart()
{
    TLOGI(WmsLogTag::DMS, "start");
    if (!displayManagerService_.Init()) {
        TLOGE(WmsLogTag::DMS, "Init failed");
        return;
    }
    if (!Publish(this)) {
        TLOGE(WmsLogTag::DMS, "Publish failed");
    }
    displayManagerService_.SetDisplayState(DisplayState::ON);
    TLOGI(WmsLogTag::DMS, "end");
}

void DisplayManagerIpcService::OnStop()
{
    TLOGI(WmsLogTag::DMS, "ready to stop display service.");
}

int DisplayManagerIpcService::Dump(int fd, const std::vector<std::u16string>& args)
{
    return displayManagerService_.Dump(fd, args);
}

ErrCode DisplayManagerIpcService::GetDefaultDisplayInfo(sptr<DisplayInfo>& displayInfo)
{
    displayInfo = displayManagerService_.GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        return ERR_INVALID_DATA;
    }
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetDisplayInfoById(uint64_t displayId, sptr<DisplayInfo>& displayInfo)
{
    displayInfo = displayManagerService_.GetDisplayInfoById(displayId);
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetDisplayInfoById failed, displayId: %{public}" PRIu64, displayId);
        return ERR_INVALID_DATA;
    }
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetVisibleAreaDisplayInfoById(uint64_t displayId, sptr<DisplayInfo>& displayInfo)
{
    displayInfo = displayManagerService_.GetVisibleAreaDisplayInfoById(displayId);
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetVisibleAreaDisplayInfoById failed, displayId: %{public}" PRIu64, displayId);
        return ERR_INVALID_DATA;
    }
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetDisplayInfoByScreen(uint64_t screenId, sptr<DisplayInfo>& displayInfo)
{
    displayInfo = displayManagerService_.GetDisplayInfoByScreen(screenId);
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetDisplayInfoByScreen failed, screenId: %{public}" PRIu64, screenId);
        return ERR_INVALID_DATA;
    }
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::CreateVirtualScreen(const DmVirtualScreenOption& virtualOption,
    const sptr<IRemoteObject>& displayManagerAgent, uint64_t& screenId)
{
    screenId = displayManagerService_.CreateVirtualScreen(virtualOption.GetOption(), displayManagerAgent);
    return  ERR_OK;
}

ErrCode DisplayManagerIpcService::CreateVirtualScreen(const DmVirtualScreenOption& virtualOption,
    const sptr<IRemoteObject>& displayManagerAgent, uint64_t& screenId,
    const sptr<IBufferProducer>& surface)
{
    VirtualScreenOption option = virtualOption.GetOption();
    sptr<IBufferProducer> surfaceCopy = surface;
    option.surface_ = Surface::CreateSurfaceAsProducer(surfaceCopy);
    screenId = displayManagerService_.CreateVirtualScreen(option, displayManagerAgent);
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::DestroyVirtualScreen(uint64_t screenId, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.DestroyVirtualScreen(screenId));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetVirtualScreenSurface(uint64_t screenId, const sptr<IBufferProducer>& surface,
    int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.SetVirtualScreenSurface(screenId, surface));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetOrientation(uint64_t screenId, uint32_t orientation, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.SetOrientation(screenId,
        static_cast<Orientation>(orientation)));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetDisplaySnapshot(uint64_t displayId, int32_t& errorCode, bool isUseDma,
    bool isCaptureFullOfScreen, std::shared_ptr<PixelMap>& pixelMap)
{
    DmErrorCode errorCodeOut = DmErrorCode::DM_OK;
    pixelMap = displayManagerService_.GetDisplaySnapshot(displayId, &errorCodeOut, isUseDma, isCaptureFullOfScreen);
    errorCode = static_cast<int32_t>(errorCodeOut);
    if (pixelMap == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetDisplaySnapshot failed, displayId: %{public}" PRIu64 ", errorCode: %{public}d",
            displayId, errorCode);
        return ERR_INVALID_DATA;
    }
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetScreenSupportedColorGamuts(uint64_t screenId, std::vector<uint32_t>& colorGamuts,
    int32_t& dmError)
{
    std::vector<ScreenColorGamut> colorGamutsOut;
    dmError = static_cast<int32_t>(displayManagerService_.GetScreenSupportedColorGamuts(screenId, colorGamutsOut));
    for (auto& colorGamut : colorGamutsOut) {
        colorGamuts.push_back(static_cast<uint32_t>(colorGamut));
    }
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetScreenColorGamut(uint64_t screenId, uint32_t& colorGamut, int32_t& dmError)
{
    ScreenColorGamut colorGamutOut;
    dmError = static_cast<int32_t>(displayManagerService_.GetScreenColorGamut(screenId, colorGamutOut));
    colorGamut = static_cast<uint32_t>(colorGamutOut);
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetScreenColorGamut(uint64_t screenId, int32_t colorGamutIdx, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.SetScreenColorGamut(screenId, colorGamutIdx));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetScreenGamutMap(uint64_t screenId, uint32_t& gamutMap, int32_t& dmError)
{
    ScreenGamutMap gamutMapOut;
    dmError = static_cast<int32_t>(displayManagerService_.GetScreenGamutMap(screenId, gamutMapOut));
    gamutMap = static_cast<uint32_t>(gamutMapOut);
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetScreenGamutMap(uint64_t screenId, uint32_t gamutMap, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.SetScreenGamutMap(screenId,
        static_cast<ScreenGamutMap>(gamutMap)));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetScreenColorTransform(uint64_t screenId)
{
    displayManagerService_.SetScreenColorTransform(screenId);
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
    uint32_t type, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.RegisterDisplayManagerAgent(displayManagerAgent,
        static_cast<DisplayManagerAgentType>(type)));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
    uint32_t type, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.UnregisterDisplayManagerAgent(displayManagerAgent,
        static_cast<DisplayManagerAgentType>(type)));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::WakeUpBegin(uint32_t reason, bool& isSucc)
{
    isSucc = displayManagerService_.WakeUpBegin(static_cast<PowerStateChangeReason>(reason));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::WakeUpEnd(bool& isSucc)
{
    isSucc = displayManagerService_.WakeUpEnd();
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SuspendBegin(uint32_t reason, bool& isSucc)
{
    isSucc = displayManagerService_.SuspendBegin(static_cast<PowerStateChangeReason>(reason));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SuspendEnd(bool& isSucc)
{
    isSucc = displayManagerService_.SuspendEnd();
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetScreenPowerForAll(uint32_t screenPowerState, uint32_t reason, bool& isSucc)
{
    isSucc = displayManagerService_.SetScreenPowerForAll(static_cast<ScreenPowerState>(screenPowerState),
        static_cast<PowerStateChangeReason>(reason));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetSpecifiedScreenPower(uint64_t screenId, uint32_t screenPowerState, uint32_t
    reason, bool& isSucc)
{
    isSucc = displayManagerService_.SetSpecifiedScreenPower(screenId, static_cast<ScreenPowerState>(screenPowerState),
        static_cast<PowerStateChangeReason>(reason));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetScreenPower(uint64_t dmsScreenId, uint32_t& screenPowerState)
{
    screenPowerState = static_cast<uint32_t>(displayManagerService_.GetScreenPower(dmsScreenId));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetDisplayState(uint32_t displayState, bool& isSucc)
{
    isSucc = displayManagerService_.SetDisplayState(static_cast<DisplayState>(displayState));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetDisplayState(uint64_t displayId, uint32_t& displayState)
{
    displayState = static_cast<uint32_t>(displayManagerService_.GetDisplayState(displayId));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::TryToCancelScreenOff(bool& isSucc)
{
    isSucc = displayManagerService_.TryToCancelScreenOff();
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetAllDisplayIds(std::vector<uint64_t>& displayIds)
{
    displayIds = displayManagerService_.GetAllDisplayIds();
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetCutoutInfo(uint64_t displayId, sptr<CutoutInfo>& cutoutInfo)
{
    cutoutInfo = displayManagerService_.GetCutoutInfo(displayId);
    if (cutoutInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetCutoutInfo failed, displayId: %{public}" PRIu64, displayId);
        return ERR_INVALID_DATA;
    }
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::AddSurfaceNodeToDisplay(uint64_t displayId,
    const std::shared_ptr<DmRsSurfaceNode>& dmRsSurfaceNode, int32_t& dmError)
{
    std::shared_ptr<RSSurfaceNode> surfaceNode = dmRsSurfaceNode->GetSurfaceNode();
    dmError = static_cast<int32_t>(displayManagerService_.AddSurfaceNodeToDisplay(displayId, surfaceNode, true));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::RemoveSurfaceNodeFromDisplay(uint64_t displayId,
    const std::shared_ptr<DmRsSurfaceNode>& dmRsSurfaceNode, int32_t& dmError)
{
    std::shared_ptr<RSSurfaceNode> surfaceNode = dmRsSurfaceNode->GetSurfaceNode();
    dmError = static_cast<int32_t>(displayManagerService_.RemoveSurfaceNodeFromDisplay(displayId, surfaceNode));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::HasPrivateWindow(uint64_t displayId, bool& hasPrivateWindow, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.HasPrivateWindow(displayId, hasPrivateWindow));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::NotifyDisplayEvent(uint32_t displayEvent)
{
    displayManagerService_.NotifyDisplayEvent(static_cast<DisplayEvent>(displayEvent));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetFreeze(const std::vector<uint64_t>& displayIds, bool isFreeze, bool& isSucc)
{
    isSucc = displayManagerService_.SetFreeze(displayIds, isFreeze);
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::MakeMirror(uint64_t mainScreenId, const std::vector<uint64_t>& mirrorScreenId,
    uint64_t& screenGroupId, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.MakeMirror(mainScreenId, mirrorScreenId, screenGroupId));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::StopMirror(const std::vector<uint64_t>& mirrorScreenIds, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.StopMirror(mirrorScreenIds));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetScreenInfoById(uint64_t screenId, sptr<ScreenInfo>& screenInfo)
{
    screenInfo = displayManagerService_.GetScreenInfoById(screenId);
    if (screenInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetScreenInfoById failed, screenId: %{public}" PRIu64, screenId);
        return ERR_INVALID_DATA;
    }
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetScreenGroupInfoById(uint64_t screenId, sptr<ScreenGroupInfo>& screenGroupInfo)
{
    screenGroupInfo = displayManagerService_.GetScreenGroupInfoById(screenId);
    if (screenGroupInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetScreenGroupInfoById failed, screenId: %{public}" PRIu64, screenId);
        return ERR_INVALID_DATA;
    }
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.GetAllScreenInfos(screenInfos));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::MakeExpand(const std::vector<uint64_t>& screenId,
    const std::vector<Point>& startPoint, uint64_t& screenGroupId, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.MakeExpand(screenId, startPoint, screenGroupId));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::StopExpand(const std::vector<uint64_t>& expandScreenIds, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.StopExpand(expandScreenIds));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::RemoveVirtualScreenFromGroup(const std::vector<uint64_t>& screens)
{
    displayManagerService_.RemoveVirtualScreenFromGroup(screens);
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetScreenActiveMode(uint64_t screenId, uint32_t modeId, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.SetScreenActiveMode(screenId, modeId));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetVirtualPixelRatio(uint64_t screenId, float virtualPixelRatio, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.SetVirtualPixelRatio(screenId, virtualPixelRatio));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetResolution(uint64_t screenId, uint32_t width, uint32_t height,
    float virtualPixelRatio, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.SetResolution(screenId, width, height, virtualPixelRatio));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetDensityInCurResolution(uint64_t screenId, float& virtualPixelRatio,
    int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.GetDensityInCurResolution(screenId, virtualPixelRatio));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::IsScreenRotationLocked(bool& isLocked, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.IsScreenRotationLocked(isLocked));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetScreenRotationLocked(bool isLocked, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.SetScreenRotationLocked(isLocked));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetScreenRotationLockedFromJs(bool isLocked, int32_t& dmError)
{
    dmError = static_cast<int32_t>(displayManagerService_.SetScreenRotationLockedFromJs(isLocked));
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetAllDisplayPhysicalResolution(
    std::vector<DisplayPhysicalResolution>& displayPhysicalResolutions)
{
    displayPhysicalResolutions = displayManagerService_.GetAllDisplayPhysicalResolution();
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetScreenBrightness(const DmsScreenBrightnessData& brightnessData, bool& isSucc)
{
    isSucc = displayManagerService_.SetScreenBrightness(brightnessData);
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::GetScreenBrightness(uint64_t screenId, uint32_t& level)
{
    level = displayManagerService_.GetScreenBrightness(screenId);
    return ERR_OK;
}

ErrCode DisplayManagerIpcService::SetVirtualScreenAsDefault(uint64_t screenId, bool& isSucc)
{
    isSucc = displayManagerService_.SetVirtualScreenAsDefault(screenId);
    return ERR_OK;
}
} // namespace OHOS::Rosen
