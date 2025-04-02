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

#include "display_manager_adapter.h"

#include <iremote_broker.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "display_manager.h"
#include "dm_common.h"
#include "scene_board_judgement.h"
#include "screen_manager.h"
#include "window_manager_hilog.h"
#include "zidl/screen_session_manager_interface.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "DisplayManagerAdapter"};
}
WM_IMPLEMENT_SINGLE_INSTANCE(DisplayManagerAdapter)
WM_IMPLEMENT_SINGLE_INSTANCE(ScreenManagerAdapter)

#define INIT_PROXY_CHECK_RETURN(ret) \
    do { \
        if (!InitDMSProxy()) { \
            WLOGFE("InitDMSProxy failed!"); \
            return ret; \
        } \
    } while (false)

sptr<DisplayInfo> DisplayManagerAdapter::GetDefaultDisplayInfo()
{
    INIT_PROXY_CHECK_RETURN(nullptr);

    return displayManagerServiceProxy_->GetDefaultDisplayInfo();
}

sptr<DisplayInfo> DisplayManagerAdapter::GetDisplayInfoByScreenId(ScreenId screenId)
{
    INIT_PROXY_CHECK_RETURN(nullptr);

    return  displayManagerServiceProxy_->GetDisplayInfoByScreen(screenId);
}

std::shared_ptr<Media::PixelMap> DisplayManagerAdapter::GetDisplaySnapshot(DisplayId displayId,
    DmErrorCode* errorCode, bool isUseDma)
{
    INIT_PROXY_CHECK_RETURN(nullptr);

    return displayManagerServiceProxy_->GetDisplaySnapshot(displayId, errorCode, isUseDma);
}

std::shared_ptr<Media::PixelMap> DisplayManagerAdapter::GetSnapshotByPicker(Media::Rect &rect, DmErrorCode* errorCode)
{
    INIT_PROXY_CHECK_RETURN(nullptr);

    return displayManagerServiceProxy_->GetSnapshotByPicker(rect, errorCode);
}

DMError ScreenManagerAdapter::GetScreenSupportedColorGamuts(ScreenId screenId,
    std::vector<ScreenColorGamut>& colorGamuts)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->GetScreenSupportedColorGamuts(screenId, colorGamuts);
}

DMError ScreenManagerAdapter::GetScreenColorGamut(ScreenId screenId, ScreenColorGamut& colorGamut)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->GetScreenColorGamut(screenId, colorGamut);
}

DMError ScreenManagerAdapter::SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetScreenColorGamut(screenId, colorGamutIdx);
}

DMError ScreenManagerAdapter::GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->GetScreenGamutMap(screenId, gamutMap);
}

DMError ScreenManagerAdapter::SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetScreenGamutMap(screenId, gamutMap);
}

DMError ScreenManagerAdapter::SetScreenColorTransform(ScreenId screenId)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetScreenColorTransform(screenId);
}

DMError ScreenManagerAdapter::GetPixelFormat(ScreenId screenId, GraphicPixelFormat& pixelFormat)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    WLOGFI("ScreenManagerAdapter::GetPixelFormat");
    return displayManagerServiceProxy_->GetPixelFormat(screenId, pixelFormat);
}

DMError ScreenManagerAdapter::SetPixelFormat(ScreenId screenId, GraphicPixelFormat pixelFormat)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    WLOGFI("ScreenManagerAdapter::SetPixelFormat");
    return displayManagerServiceProxy_->SetPixelFormat(screenId, pixelFormat);
}

DMError ScreenManagerAdapter::GetSupportedHDRFormats(ScreenId screenId,
    std::vector<ScreenHDRFormat>& hdrFormats)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    WLOGFI("ScreenManagerAdapter::GetSupportedHDRFormats");
    return displayManagerServiceProxy_->GetSupportedHDRFormats(screenId, hdrFormats);
}

DMError ScreenManagerAdapter::GetScreenHDRFormat(ScreenId screenId, ScreenHDRFormat& hdrFormat)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    WLOGFI("ScreenManagerAdapter::GetScreenHDRFormat");
    return displayManagerServiceProxy_->GetScreenHDRFormat(screenId, hdrFormat);
}

DMError ScreenManagerAdapter::SetScreenHDRFormat(ScreenId screenId, int32_t modeIdx)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    WLOGFI("ScreenManagerAdapter::SetScreenHDRFormat");
    return displayManagerServiceProxy_->SetScreenHDRFormat(screenId, modeIdx);
}

DMError ScreenManagerAdapter::GetSupportedColorSpaces(ScreenId screenId,
    std::vector<GraphicCM_ColorSpaceType>& colorSpaces)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    WLOGFI("ScreenManagerAdapter::GetSupportedColorSpaces");
    return displayManagerServiceProxy_->GetSupportedColorSpaces(screenId, colorSpaces);
}

DMError ScreenManagerAdapter::GetScreenColorSpace(ScreenId screenId,
    GraphicCM_ColorSpaceType& colorSpace)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    WLOGFI("ScreenManagerAdapter::GetScreenColorSpace");
    return displayManagerServiceProxy_->GetScreenColorSpace(screenId, colorSpace);
}

DMError ScreenManagerAdapter::SetScreenColorSpace(ScreenId screenId,
    GraphicCM_ColorSpaceType colorSpace)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    WLOGFI("ScreenManagerAdapter::SetScreenColorSpace");
    return displayManagerServiceProxy_->SetScreenColorSpace(screenId, colorSpace);
}

DMError ScreenManagerAdapter::GetSupportedHDRFormats(ScreenId screenId, std::vector<uint32_t>& hdrFormats)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    std::vector<ScreenHDRFormat> hdrFormatsVec;
    DMError ret = GetSupportedHDRFormats(screenId, hdrFormatsVec);
    for (auto value : hdrFormatsVec) {
        hdrFormats.push_back(static_cast<uint32_t>(value));
    }
    WLOGFI("ScreenManagerAdapter::GetSupportedHDRFormats ret %{public}d", static_cast<int32_t>(ret));
    return ret;
}

DMError ScreenManagerAdapter::GetSupportedColorSpaces(ScreenId screenId, std::vector<uint32_t>& colorSpaces)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    std::vector<GraphicCM_ColorSpaceType> colorSpacesVec;
    DMError ret = GetSupportedColorSpaces(screenId, colorSpacesVec);
    for (auto value : colorSpacesVec) {
        colorSpaces.push_back(static_cast<uint32_t>(value));
    }
    WLOGFI("ScreenManagerAdapter::GetSupportedColorSpaces ret %{public}d", static_cast<int32_t>(ret));
    return ret;
}

ScreenId ScreenManagerAdapter::CreateVirtualScreen(VirtualScreenOption option,
    const sptr<IDisplayManagerAgent>& displayManagerAgent)
{
    INIT_PROXY_CHECK_RETURN(SCREEN_ID_INVALID);

    WLOGFI("DisplayManagerAdapter::CreateVirtualScreen");
    return displayManagerServiceProxy_->CreateVirtualScreen(option, displayManagerAgent->AsObject());
}

DMError ScreenManagerAdapter::DestroyVirtualScreen(ScreenId screenId)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    WLOGFI("DisplayManagerAdapter::DestroyVirtualScreen");
    return displayManagerServiceProxy_->DestroyVirtualScreen(screenId);
}

DMError ScreenManagerAdapter::SetVirtualScreenSurface(ScreenId screenId, sptr<Surface> surface)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (surface == nullptr) {
        WLOGFE("Surface is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    WLOGFI("DisplayManagerAdapter::SetVirtualScreenSurface");
    return displayManagerServiceProxy_->SetVirtualScreenSurface(screenId, surface->GetProducer());
}

DMError ScreenManagerAdapter::AddVirtualScreenBlockList(const std::vector<int32_t>& persistentIds)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    return displayManagerServiceProxy_->AddVirtualScreenBlockList(persistentIds);
}

DMError ScreenManagerAdapter::RemoveVirtualScreenBlockList(const std::vector<int32_t>& persistentIds)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    return displayManagerServiceProxy_->RemoveVirtualScreenBlockList(persistentIds);
}

DMError ScreenManagerAdapter::SetScreenPrivacyMaskImage(ScreenId screenId,
    const std::shared_ptr<Media::PixelMap>& privacyMaskImg)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    WLOGFI("DisplayManagerAdapter::SetScreenPrivacyMaskImage");
    return displayManagerServiceProxy_->SetScreenPrivacyMaskImage(screenId, privacyMaskImg);
}

DMError ScreenManagerAdapter::SetVirtualMirrorScreenCanvasRotation(ScreenId screenId, bool canvasRotation)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    WLOGFI("DisplayManagerAdapter::SetVirtualMirrorScreenCanvasRotation");
    return displayManagerServiceProxy_->SetVirtualMirrorScreenCanvasRotation(screenId, canvasRotation);
}

DMError ScreenManagerAdapter::SetVirtualMirrorScreenScaleMode(ScreenId screenId, ScreenScaleMode scaleMode)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    WLOGFI("DisplayManagerAdapter::SetVirtualMirrorScreenScaleMode");
    return displayManagerServiceProxy_->SetVirtualMirrorScreenScaleMode(screenId, scaleMode);
}

DMError ScreenManagerAdapter::SetScreenRotationLocked(bool isLocked)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    WLOGFI("DisplayManagerAdapter::SetScreenRotationLocked");
    return displayManagerServiceProxy_->SetScreenRotationLocked(isLocked);
}

DMError ScreenManagerAdapter::SetScreenRotationLockedFromJs(bool isLocked)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    WLOGFI("DisplayManagerAdapter::SetScreenRotationLockedFromJs");
    return displayManagerServiceProxy_->SetScreenRotationLockedFromJs(isLocked);
}

DMError ScreenManagerAdapter::IsScreenRotationLocked(bool& isLocked)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    WLOGFI("DisplayManagerAdapter::IsScreenRotationLocked");
    return displayManagerServiceProxy_->IsScreenRotationLocked(isLocked);
}

bool ScreenManagerAdapter::SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);
    return displayManagerServiceProxy_->SetSpecifiedScreenPower(screenId, state, reason);
}

bool ScreenManagerAdapter::SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);
    return displayManagerServiceProxy_->SetScreenPowerForAll(state, reason);
}

ScreenPowerState ScreenManagerAdapter::GetScreenPower(ScreenId dmsScreenId)
{
    INIT_PROXY_CHECK_RETURN(ScreenPowerState::INVALID_STATE);
    return displayManagerServiceProxy_->GetScreenPower(dmsScreenId);
}

ScreenPowerState ScreenManagerAdapter::GetScreenPower()
{
    INIT_PROXY_CHECK_RETURN(ScreenPowerState::INVALID_STATE);
    return displayManagerServiceProxy_->GetScreenPower();
}

DMError ScreenManagerAdapter::SetOrientation(ScreenId screenId, Orientation orientation)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetOrientation(screenId, orientation);
}

DMError BaseAdapter::RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->RegisterDisplayManagerAgent(displayManagerAgent, type);
}

DMError BaseAdapter::UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->UnregisterDisplayManagerAgent(displayManagerAgent, type);
}

bool DisplayManagerAdapter::WakeUpBegin(PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->WakeUpBegin(reason);
}

bool DisplayManagerAdapter::WakeUpEnd()
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->WakeUpEnd();
}

bool DisplayManagerAdapter::SuspendBegin(PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->SuspendBegin(reason);
}

bool DisplayManagerAdapter::SuspendEnd()
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->SuspendEnd();
}

ScreenId DisplayManagerAdapter::GetInternalScreenId()
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->GetInternalScreenId();
}

bool DisplayManagerAdapter::SetScreenPowerById(ScreenId screenId, ScreenPowerState state,
    PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);
    
    return displayManagerServiceProxy_->SetScreenPowerById(screenId, state, reason);
}

bool DisplayManagerAdapter::SetDisplayState(DisplayState state)
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->SetDisplayState(state);
}

DisplayState DisplayManagerAdapter::GetDisplayState(DisplayId displayId)
{
    INIT_PROXY_CHECK_RETURN(DisplayState::UNKNOWN);

    return displayManagerServiceProxy_->GetDisplayState(displayId);
}

bool DisplayManagerAdapter::TryToCancelScreenOff()
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->TryToCancelScreenOff();
}

void DisplayManagerAdapter::NotifyDisplayEvent(DisplayEvent event)
{
    INIT_PROXY_CHECK_RETURN();

    displayManagerServiceProxy_->NotifyDisplayEvent(event);
}

bool DisplayManagerAdapter::SetFreeze(std::vector<DisplayId> displayIds, bool isFreeze)
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->SetFreeze(displayIds, isFreeze);
}

bool BaseAdapter::InitDMSProxy()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!isProxyValid_) {
        sptr<ISystemAbilityManager> systemAbilityManager =
                SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityManager) {
            WLOGFE("Failed to get system ability mgr.");
            return false;
        }

        sptr<IRemoteObject> remoteObject
            = systemAbilityManager->GetSystemAbility(DISPLAY_MANAGER_SERVICE_SA_ID);
        if (!remoteObject) {
            WLOGFE("Failed to get display manager service.");
            return false;
        }

        if (SceneBoardJudgement::IsSceneBoardEnabled()) {
            displayManagerServiceProxy_ = iface_cast<IScreenSessionManager>(remoteObject);
        } else {
            displayManagerServiceProxy_ = iface_cast<IDisplayManager>(remoteObject);
        }
        if ((!displayManagerServiceProxy_) || (!displayManagerServiceProxy_->AsObject())) {
            WLOGFW("Failed to get system display manager services");
            return false;
        }

        dmsDeath_ = new(std::nothrow) DMSDeathRecipient(*this);
        if (dmsDeath_ == nullptr) {
            WLOGFE("Failed to create death Recipient ptr DMSDeathRecipient");
            return false;
        }
        if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(dmsDeath_)) {
            WLOGFE("Failed to add death recipient");
            return false;
        }
        isProxyValid_ = true;
    }
    return true;
}

DMSDeathRecipient::DMSDeathRecipient(BaseAdapter& adapter) : adapter_(adapter)
{
}

void DMSDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    if (wptrDeath == nullptr) {
        WLOGFE("wptrDeath is null");
        return;
    }

    sptr<IRemoteObject> object = wptrDeath.promote();
    if (!object) {
        WLOGFE("object is null");
        return;
    }
    WLOGFI("dms OnRemoteDied");
    adapter_.Clear();
    if (SingletonContainer::IsDestroyed()) {
        WLOGFE("SingletonContainer is destroyed");
        return;
    }
    SingletonContainer::Get<DisplayManager>().OnRemoteDied();
    SingletonContainer::Get<ScreenManager>().OnRemoteDied();
    return;
}


BaseAdapter::~BaseAdapter()
{
    WLOGFI("destory!");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    Clear();
    displayManagerServiceProxy_ = nullptr;
}

void BaseAdapter::Clear()
{
    WLOGFD("Clear!");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if ((displayManagerServiceProxy_ != nullptr) && (displayManagerServiceProxy_->AsObject() != nullptr)) {
        displayManagerServiceProxy_->AsObject()->RemoveDeathRecipient(dmsDeath_);
    }
    isProxyValid_ = false;
}

DMError ScreenManagerAdapter::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId,
    ScreenId& screenGroupId)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->MakeMirror(mainScreenId, mirrorScreenId, screenGroupId);
}

DMError ScreenManagerAdapter::MakeMirrorForRecord(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId,
    ScreenId& screenGroupId)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->MakeMirrorForRecord(mainScreenId, mirrorScreenId, screenGroupId);
}

DMError ScreenManagerAdapter::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId,
    DMRect mainScreenRegion, ScreenId& screenGroupId)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->MakeMirror(mainScreenId, mirrorScreenId, mainScreenRegion, screenGroupId);
}

DMError ScreenManagerAdapter::SetMultiScreenMode(ScreenId mainScreenId, ScreenId secondaryScreenId,
    MultiScreenMode screenMode)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetMultiScreenMode(mainScreenId, secondaryScreenId, screenMode);
}

DMError ScreenManagerAdapter::SetMultiScreenRelativePosition(MultiScreenPositionOptions mainScreenOptions,
    MultiScreenPositionOptions secondScreenOption)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetMultiScreenRelativePosition(mainScreenOptions, secondScreenOption);
}

DMError ScreenManagerAdapter::StopMirror(const std::vector<ScreenId>& mirrorScreenIds)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->StopMirror(mirrorScreenIds);
}

DMError ScreenManagerAdapter::DisableMirror(bool disableOrNot)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->DisableMirror(disableOrNot);
}

sptr<ScreenInfo> ScreenManagerAdapter::GetScreenInfo(ScreenId screenId)
{
    if (screenId == SCREEN_ID_INVALID) {
        WLOGFE("screen id is invalid");
        return nullptr;
    }
    INIT_PROXY_CHECK_RETURN(nullptr);

    sptr<ScreenInfo> screenInfo = displayManagerServiceProxy_->GetScreenInfoById(screenId);
    return screenInfo;
}

std::vector<DisplayId> DisplayManagerAdapter::GetAllDisplayIds()
{
    WLOGFD("DisplayManagerAdapter::GetAllDisplayIds enter");
    INIT_PROXY_CHECK_RETURN(std::vector<DisplayId>());

    return displayManagerServiceProxy_->GetAllDisplayIds();
}

DMError DisplayManagerAdapter::HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->HasPrivateWindow(displayId, hasPrivateWindow);
}

bool DisplayManagerAdapter::ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId)
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->ConvertScreenIdToRsScreenId(screenId, rsScreenId);
}

DMError DisplayManagerAdapter::HasImmersiveWindow(ScreenId screenId, bool& immersive)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->HasImmersiveWindow(screenId, immersive);
}

sptr<DisplayInfo> DisplayManagerAdapter::GetDisplayInfo(DisplayId displayId)
{
    WLOGFD("DisplayManagerAdapter::GetDisplayInfo enter, displayId: %{public}" PRIu64" ", displayId);
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGFE("screen id is invalid");
        return nullptr;
    }
    INIT_PROXY_CHECK_RETURN(nullptr);

    return displayManagerServiceProxy_->GetDisplayInfoById(displayId);
}

sptr<DisplayInfo> DisplayManagerAdapter::GetVisibleAreaDisplayInfoById(DisplayId displayId)
{
    WLOGFD("enter, displayId: %{public}" PRIu64" ", displayId);
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGFE("display id is invalid");
        return nullptr;
    }
    INIT_PROXY_CHECK_RETURN(nullptr);

    return displayManagerServiceProxy_->GetVisibleAreaDisplayInfoById(displayId);
}

sptr<CutoutInfo> DisplayManagerAdapter::GetCutoutInfo(DisplayId displayId)
{
    WLOGFD("DisplayManagerAdapter::GetCutoutInfo");
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGFE("screen id is invalid");
        return nullptr;
    }
    INIT_PROXY_CHECK_RETURN(nullptr);
    return displayManagerServiceProxy_->GetCutoutInfo(displayId);
}

DMError DisplayManagerAdapter::AddSurfaceNodeToDisplay(DisplayId displayId,
    std::shared_ptr<class RSSurfaceNode>& surfaceNode)
{
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGFE("screen id is invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    return displayManagerServiceProxy_->AddSurfaceNodeToDisplay(displayId, surfaceNode);
}

DMError DisplayManagerAdapter::RemoveSurfaceNodeFromDisplay(DisplayId displayId,
    std::shared_ptr<class RSSurfaceNode>& surfaceNode)
{
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGFE("screen id is invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    return displayManagerServiceProxy_->RemoveSurfaceNodeFromDisplay(displayId, surfaceNode);
}

bool DisplayManagerAdapter::IsFoldable()
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->IsFoldable();
}

bool DisplayManagerAdapter::IsCaptured()
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->IsCaptured();
}

FoldStatus DisplayManagerAdapter::GetFoldStatus()
{
    INIT_PROXY_CHECK_RETURN(FoldStatus::UNKNOWN);

    return displayManagerServiceProxy_->GetFoldStatus();
}

FoldDisplayMode DisplayManagerAdapter::GetFoldDisplayMode()
{
    INIT_PROXY_CHECK_RETURN(FoldDisplayMode::UNKNOWN);

    return displayManagerServiceProxy_->GetFoldDisplayMode();
}

void DisplayManagerAdapter::SetFoldDisplayMode(const FoldDisplayMode mode)
{
    INIT_PROXY_CHECK_RETURN();

    return displayManagerServiceProxy_->SetFoldDisplayMode(mode);
}

DMError DisplayManagerAdapter::SetFoldDisplayModeFromJs(const FoldDisplayMode mode, std::string reason)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetFoldDisplayModeFromJs(mode, reason);
}

void DisplayManagerAdapter::SetDisplayScale(ScreenId screenId,
    float scaleX, float scaleY, float pivotX, float pivotY)
{
    INIT_PROXY_CHECK_RETURN();

    return displayManagerServiceProxy_->SetDisplayScale(screenId, scaleX, scaleY, pivotX, pivotY);
}

void DisplayManagerAdapter::SetFoldStatusLocked(bool locked)
{
    INIT_PROXY_CHECK_RETURN();

    return displayManagerServiceProxy_->SetFoldStatusLocked(locked);
}

DMError DisplayManagerAdapter::SetFoldStatusLockedFromJs(bool locked)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetFoldStatusLockedFromJs(locked);
}

sptr<FoldCreaseRegion> DisplayManagerAdapter::GetCurrentFoldCreaseRegion()
{
    INIT_PROXY_CHECK_RETURN(nullptr);

    return displayManagerServiceProxy_->GetCurrentFoldCreaseRegion();
}

sptr<ScreenGroupInfo> ScreenManagerAdapter::GetScreenGroupInfoById(ScreenId screenId)
{
    if (screenId == SCREEN_ID_INVALID) {
        WLOGFE("screenGroup id is invalid");
        return nullptr;
    }
    INIT_PROXY_CHECK_RETURN(nullptr);

    return displayManagerServiceProxy_->GetScreenGroupInfoById(screenId);
}

DMError ScreenManagerAdapter::GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->GetAllScreenInfos(screenInfos);
}

DMError ScreenManagerAdapter::MakeExpand(std::vector<ScreenId> screenId, std::vector<Point> startPoint,
                                         ScreenId& screenGroupId)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->MakeExpand(screenId, startPoint, screenGroupId);
}

DMError ScreenManagerAdapter::StopExpand(const std::vector<ScreenId>& expandScreenIds)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->StopExpand(expandScreenIds);
}


void ScreenManagerAdapter::RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens)
{
    INIT_PROXY_CHECK_RETURN();

    displayManagerServiceProxy_->RemoveVirtualScreenFromGroup(screens);
}

DMError ScreenManagerAdapter::SetScreenActiveMode(ScreenId screenId, uint32_t modeId)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetScreenActiveMode(screenId, modeId);
}

DMError ScreenManagerAdapter::SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetVirtualPixelRatio(screenId, virtualPixelRatio);
}

DMError ScreenManagerAdapter::SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetVirtualPixelRatioSystem(screenId, virtualPixelRatio);
}

DMError ScreenManagerAdapter::SetDefaultDensityDpi(ScreenId screenId, float virtualPixelRatio)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetDefaultDensityDpi(screenId, virtualPixelRatio);
}

DMError ScreenManagerAdapter::SetResolution(ScreenId screenId, uint32_t width, uint32_t height, float virtualPixelRatio)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetResolution(screenId, width, height, virtualPixelRatio);
}

DMError ScreenManagerAdapter::GetDensityInCurResolution(ScreenId screenId, float& virtualPixelRatio)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->GetDensityInCurResolution(screenId, virtualPixelRatio);
}

DMError ScreenManagerAdapter::ResizeVirtualScreen(ScreenId screenId, uint32_t width, uint32_t height)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->ResizeVirtualScreen(screenId, width, height);
}

DMError ScreenManagerAdapter::MakeUniqueScreen(const std::vector<ScreenId>& screenIds,
    std::vector<DisplayId>& displayIds)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->MakeUniqueScreen(screenIds, displayIds);
}

DMError DisplayManagerAdapter::GetAvailableArea(DisplayId displayId, DMRect& area)
{
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGFE("displayId id is invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->GetAvailableArea(displayId, area);
}

DMError DisplayManagerAdapter::GetExpandAvailableArea(DisplayId displayId, DMRect& area)
{
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGFE("displayId id is invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->GetExpandAvailableArea(displayId, area);
}

VirtualScreenFlag ScreenManagerAdapter::GetVirtualScreenFlag(ScreenId screenId)
{
    INIT_PROXY_CHECK_RETURN(VirtualScreenFlag::DEFAULT);
    if (screenId == SCREEN_ID_INVALID) {
        WLOGFE("screenId id is invalid");
        return VirtualScreenFlag::DEFAULT;
    }

    return displayManagerServiceProxy_->GetVirtualScreenFlag(screenId);
}

DMError ScreenManagerAdapter::SetVirtualScreenFlag(ScreenId screenId, VirtualScreenFlag screenFlag)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenId == SCREEN_ID_INVALID) {
        WLOGFE("displayId id is invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (screenFlag < VirtualScreenFlag::DEFAULT || screenFlag >= VirtualScreenFlag::MAX) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return displayManagerServiceProxy_->SetVirtualScreenFlag(screenId, screenFlag);
}

DMError ScreenManagerAdapter::SetVirtualScreenRefreshRate(ScreenId screenId, uint32_t refreshInterval)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->SetVirtualScreenRefreshRate(screenId, refreshInterval);
}

void DisplayManagerAdapter::SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList,
    std::vector<uint64_t> surfaceIdList)
{
    INIT_PROXY_CHECK_RETURN();
    displayManagerServiceProxy_->SetVirtualScreenBlackList(screenId, windowIdList, surfaceIdList);
}

void DisplayManagerAdapter::SetVirtualDisplayMuteFlag(ScreenId screenId, bool muteFlag)
{
    INIT_PROXY_CHECK_RETURN();
    displayManagerServiceProxy_->SetVirtualDisplayMuteFlag(screenId, muteFlag);
}

void DisplayManagerAdapter::DisablePowerOffRenderControl(ScreenId screenId)
{
    INIT_PROXY_CHECK_RETURN();
    displayManagerServiceProxy_->DisablePowerOffRenderControl(screenId);
}

DMError DisplayManagerAdapter::ProxyForFreeze(const std::set<int32_t>& pidList, bool isProxy)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    return displayManagerServiceProxy_->ProxyForFreeze(pidList, isProxy);
}

DMError DisplayManagerAdapter::ResetAllFreezeStatus()
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    return displayManagerServiceProxy_->ResetAllFreezeStatus();
}

std::vector<DisplayPhysicalResolution> DisplayManagerAdapter::GetAllDisplayPhysicalResolution()
{
    INIT_PROXY_CHECK_RETURN(std::vector<DisplayPhysicalResolution>{});
    return displayManagerServiceProxy_->GetAllDisplayPhysicalResolution();
}

std::string DisplayManagerAdapter::GetDisplayCapability()
{
    INIT_PROXY_CHECK_RETURN("");
    return displayManagerServiceProxy_->GetDisplayCapability();
}

DMError DisplayManagerAdapter::SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid,
    std::vector<uint64_t>& windowIdList)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    return displayManagerServiceProxy_->SetVirtualScreenSecurityExemption(screenId, pid, windowIdList);
}

bool ScreenManagerAdapter::SetVirtualScreenStatus(ScreenId screenId, VirtualScreenStatus screenStatus)
{
    INIT_PROXY_CHECK_RETURN(false);
    return displayManagerServiceProxy_->SetVirtualScreenStatus(screenId, screenStatus);
}

DMError ScreenManagerAdapter::SetVirtualScreenMaxRefreshRate(ScreenId id, uint32_t refreshRate,
    uint32_t& actualRefreshRate)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    return displayManagerServiceProxy_->SetVirtualScreenMaxRefreshRate(id, refreshRate, actualRefreshRate);
}

std::shared_ptr<Media::PixelMap> DisplayManagerAdapter::GetScreenCapture(const CaptureOption& captureOption,
    DmErrorCode* errorCode)
{
    INIT_PROXY_CHECK_RETURN(nullptr);
    return displayManagerServiceProxy_->GetScreenCapture(captureOption, errorCode);
}

sptr<DisplayInfo> DisplayManagerAdapter::GetPrimaryDisplayInfo()
{
    INIT_PROXY_CHECK_RETURN(nullptr);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        return displayManagerServiceProxy_->GetPrimaryDisplayInfo();
    } else {
        return displayManagerServiceProxy_->GetDefaultDisplayInfo();
    }
}

std::shared_ptr<Media::PixelMap> DisplayManagerAdapter::GetDisplaySnapshotWithOption(const CaptureOption& captureOption,
    DmErrorCode* errorCode)
{
    INIT_PROXY_CHECK_RETURN(nullptr);
    return displayManagerServiceProxy_->GetDisplaySnapshotWithOption(captureOption, errorCode);
}

DMError ScreenManagerAdapter::SetScreenSkipProtectedWindow(const std::vector<ScreenId>& screenIds, bool isEnable)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    return displayManagerServiceProxy_->SetScreenSkipProtectedWindow(screenIds, isEnable);
}
} // namespace OHOS::Rosen
