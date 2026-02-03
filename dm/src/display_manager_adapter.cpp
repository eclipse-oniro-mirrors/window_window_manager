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
#include "dms_global_mutex.h"

namespace OHOS::Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(DisplayManagerAdapter)
WM_IMPLEMENT_SINGLE_INSTANCE(ScreenManagerAdapter)

#define INIT_PROXY_CHECK_RETURN(ret) \
    do { \
        if (!InitDMSProxy()) { \
            TLOGE(WmsLogTag::DMS, "InitDMSProxy failed!"); \
            return ret; \
        } \
    } while (false)

DMError BaseAdapter::ConvertToDMError(ErrCode errCode, int32_t dmError)
{
    if (FAILED(errCode)) {
        TLOGE(WmsLogTag::DMS, "ConvertToDMError errCode: %{public}d, dmError: %{public}d", errCode, dmError);
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(dmError);
}

sptr<DisplayInfo> DisplayManagerAdapter::GetDefaultDisplayInfo(int32_t userId)
{
    INIT_PROXY_CHECK_RETURN(nullptr);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetDefaultDisplayInfo(userId);
    }

    sptr<DisplayInfo> displayInfo;
    ErrCode errCode = displayManagerServiceProxy_->GetDefaultDisplayInfo(displayInfo);
    if (FAILED(errCode) || displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "failed, errCode: %{public}d, displayInfo: %{public}s", errCode,
            displayInfo == nullptr ? "null" : "not null");
        return nullptr;
    }
    return displayInfo;
}

bool DisplayManagerAdapter::SetVirtualScreenAsDefault(ScreenId screenId)
{
    INIT_PROXY_CHECK_RETURN(false);
    bool isSuccess = false;
    if (displayManagerServiceProxy_) {
        displayManagerServiceProxy_->SetVirtualScreenAsDefault(screenId, isSuccess);
    }
    return isSuccess;
}

sptr<DisplayInfo> DisplayManagerAdapter::GetDisplayInfoByScreenId(ScreenId screenId)
{
    INIT_PROXY_CHECK_RETURN(nullptr);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetDisplayInfoByScreen(screenId);
    }

    sptr<DisplayInfo> displayInfo;
    ErrCode errCode = displayManagerServiceProxy_->GetDisplayInfoByScreen(screenId, displayInfo);
    if (FAILED(errCode) || displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "failed, screenId: %{public}" PRIu64 ", errCode: %{public}d, displayInfo: %{public}s",
            screenId, errCode, displayInfo == nullptr ? "null" : "not null");
        return nullptr;
    }
    return displayInfo;
}

std::shared_ptr<Media::PixelMap> DisplayManagerAdapter::GetDisplaySnapshot(DisplayId displayId,
    DmErrorCode* errorCode, bool isUseDma, bool isCaptureFullOfScreen)
{
    INIT_PROXY_CHECK_RETURN(nullptr);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetDisplaySnapshot(displayId, errorCode, isUseDma,
            isCaptureFullOfScreen);
    }

    auto errorCodeOut = static_cast<int32_t>(DmErrorCode::DM_OK);
    std::shared_ptr<Media::PixelMap> pixelMap;
    ErrCode errCode = displayManagerServiceProxy_->GetDisplaySnapshot(displayId, errorCodeOut, isUseDma,
        isCaptureFullOfScreen, pixelMap);
    if (errorCode != nullptr) {
        *errorCode = static_cast<DmErrorCode>(errorCodeOut);
    }
    if (FAILED(errCode) || pixelMap == nullptr) {
        TLOGE(WmsLogTag::DMS, "failed, displayId: %{public}" PRIu64 ", errCode: %{public}d, pixelMap: %{public}s",
            displayId, errCode, pixelMap == nullptr ? "null" : "not null");
        return nullptr;
    }
    return pixelMap;
}

std::vector<std::shared_ptr<Media::PixelMap>> DisplayManagerAdapter::GetDisplayHDRSnapshot(DisplayId displayId,
    DmErrorCode& errorCode, bool isUseDma, bool isCaptureFullOfScreen)
{
    std::vector<std::shared_ptr<Media::PixelMap>> ret = { nullptr, nullptr };
    INIT_PROXY_CHECK_RETURN(ret);
 
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetDisplayHDRSnapshot(displayId, errorCode, isUseDma,
            isCaptureFullOfScreen);
    }
    errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
    return { nullptr, nullptr };
}

std::shared_ptr<Media::PixelMap> DisplayManagerAdapter::GetSnapshotByPicker(Media::Rect& rect, DmErrorCode* errorCode)
{
    INIT_PROXY_CHECK_RETURN(nullptr);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetSnapshotByPicker(rect, errorCode);
    }

    if (errorCode != nullptr) {
        *errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    return nullptr;
}

DMError ScreenManagerAdapter::GetScreenSupportedColorGamuts(ScreenId screenId,
    std::vector<ScreenColorGamut>& colorGamuts)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetScreenSupportedColorGamuts(screenId, colorGamuts);
    }

    std::vector<uint32_t> colorGamutsOut;
    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->GetScreenSupportedColorGamuts(screenId, colorGamutsOut, dmError);
    if (SUCCEEDED(errCode)) {
        for (auto colorGamut : colorGamutsOut) {
            colorGamuts.push_back(static_cast<ScreenColorGamut>(colorGamut));
        }
    }
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::GetScreenColorGamut(ScreenId screenId, ScreenColorGamut& colorGamut)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetScreenColorGamut(screenId, colorGamut);
    }

    uint32_t colorGamutOut;
    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->GetScreenColorGamut(screenId, colorGamutOut, dmError);
    if (SUCCEEDED(errCode)) {
        colorGamut = static_cast<ScreenColorGamut>(colorGamutOut);
    }
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetScreenColorGamut(screenId, colorGamutIdx);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->SetScreenColorGamut(screenId, colorGamutIdx, dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetScreenGamutMap(screenId, gamutMap);
    }

    uint32_t gamutMapOut;
    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->GetScreenGamutMap(screenId, gamutMapOut, dmError);
    if (SUCCEEDED(errCode)) {
        gamutMap = static_cast<ScreenGamutMap>(gamutMapOut);
    }
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetScreenGamutMap(screenId, gamutMap);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->SetScreenGamutMap(screenId, static_cast<uint32_t>(gamutMap),
        dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::SetScreenColorTransform(ScreenId screenId)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetScreenColorTransform(screenId);
    }

    ErrCode errCode = displayManagerServiceProxy_->SetScreenColorTransform(screenId);
    return ConvertToDMError(errCode, static_cast<int32_t>(DMError::DM_OK));
}

DMError ScreenManagerAdapter::GetPixelFormat(ScreenId screenId, GraphicPixelFormat& pixelFormat)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetPixelFormat(screenId, pixelFormat);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::SetPixelFormat(ScreenId screenId, GraphicPixelFormat pixelFormat)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetPixelFormat(screenId, pixelFormat);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::GetSupportedHDRFormats(ScreenId screenId,
    std::vector<ScreenHDRFormat>& hdrFormats)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetSupportedHDRFormats(screenId, hdrFormats);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::GetScreenHDRFormat(ScreenId screenId, ScreenHDRFormat& hdrFormat)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetScreenHDRFormat(screenId, hdrFormat);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::SetScreenHDRFormat(ScreenId screenId, int32_t modeIdx)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetScreenHDRFormat(screenId, modeIdx);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::GetSupportedColorSpaces(ScreenId screenId,
    std::vector<GraphicCM_ColorSpaceType>& colorSpaces)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetSupportedColorSpaces(screenId, colorSpaces);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::GetScreenColorSpace(ScreenId screenId,
    GraphicCM_ColorSpaceType& colorSpace)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetScreenColorSpace(screenId, colorSpace);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::SetScreenColorSpace(ScreenId screenId,
    GraphicCM_ColorSpaceType colorSpace)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetScreenColorSpace(screenId, colorSpace);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::GetSupportedHDRFormats(ScreenId screenId, std::vector<uint32_t>& hdrFormats)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    std::vector<ScreenHDRFormat> hdrFormatsVec;
    DMError ret = GetSupportedHDRFormats(screenId, hdrFormatsVec);
    for (auto value : hdrFormatsVec) {
        hdrFormats.push_back(static_cast<uint32_t>(value));
    }
    TLOGI(WmsLogTag::DMS, "ret %{public}d", static_cast<int32_t>(ret));
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
    TLOGI(WmsLogTag::DMS, "ret %{public}d", static_cast<int32_t>(ret));
    return ret;
}

ScreenId ScreenManagerAdapter::CreateVirtualScreen(VirtualScreenOption option,
    const sptr<IDisplayManagerAgent>& displayManagerAgent)
{
    INIT_PROXY_CHECK_RETURN(SCREEN_ID_INVALID);

    if (displayManagerAgent == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayManagerAgent is nullptr");
        return SCREEN_ID_INVALID;
    }

    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->CreateVirtualScreen(option, displayManagerAgent->AsObject());
    }

    ErrCode errCode;
    ScreenId screenId = SCREEN_ID_INVALID;
    DmVirtualScreenOption dmVirtualScreenOption(option);
    if (option.surface_ && option.surface_->GetProducer()) {
        errCode = displayManagerServiceProxy_->CreateVirtualScreen(dmVirtualScreenOption,
            displayManagerAgent->AsObject(), screenId, option.surface_->GetProducer());
    } else {
        errCode = displayManagerServiceProxy_->CreateVirtualScreen(dmVirtualScreenOption,
            displayManagerAgent->AsObject(), screenId);
    }
    if (FAILED(errCode)) {
        TLOGE(WmsLogTag::DMS, "failed, errCode: %{public}d", errCode);
        return SCREEN_ID_INVALID;
    }
    return screenId;
}

DMError ScreenManagerAdapter::DestroyVirtualScreen(ScreenId screenId, bool isCallingByThirdParty)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->DestroyVirtualScreen(screenId, isCallingByThirdParty);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->DestroyVirtualScreen(screenId, dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::SetVirtualScreenSurface(ScreenId screenId, sptr<Surface> surface)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (surface == nullptr) {
        TLOGE(WmsLogTag::DMS, "Surface is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetVirtualScreenSurface(screenId, surface->GetProducer());
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->SetVirtualScreenSurface(screenId, surface->GetProducer(),
        dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::AddVirtualScreenBlockList(const std::vector<int32_t>& persistentIds)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->AddVirtualScreenBlockList(persistentIds);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::RemoveVirtualScreenBlockList(const std::vector<int32_t>& persistentIds)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->RemoveVirtualScreenBlockList(persistentIds);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::AddVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->AddVirtualScreenWhiteList(screenId, missionIds);
    }
    TLOGE(WmsLogTag::DMS, "Device not support");
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::RemoveVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->RemoveVirtualScreenWhiteList(screenId, missionIds);
    }
    TLOGE(WmsLogTag::DMS, "Device not support");
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::SetScreenPrivacyMaskImage(ScreenId screenId,
    const std::shared_ptr<Media::PixelMap>& privacyMaskImg)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetScreenPrivacyMaskImage(screenId, privacyMaskImg);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::SetVirtualMirrorScreenCanvasRotation(ScreenId screenId, bool canvasRotation)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetVirtualMirrorScreenCanvasRotation(screenId, canvasRotation);
    }

    return DMError::DM_OK;
}

DMError ScreenManagerAdapter::SetVirtualMirrorScreenScaleMode(ScreenId screenId, ScreenScaleMode scaleMode)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetVirtualMirrorScreenScaleMode(screenId, scaleMode);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::SetScreenRotationLocked(bool isLocked)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetScreenRotationLocked(isLocked);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->SetScreenRotationLocked(isLocked, dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::SetScreenRotationLockedFromJs(bool isLocked)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetScreenRotationLockedFromJs(isLocked);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->SetScreenRotationLockedFromJs(isLocked, dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::IsScreenRotationLocked(bool& isLocked)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->IsScreenRotationLocked(isLocked);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->IsScreenRotationLocked(isLocked, dmError);
    return ConvertToDMError(errCode, dmError);
}

bool ScreenManagerAdapter::SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state,
    PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetSpecifiedScreenPower(screenId, state, reason);
    }

    bool isSucc = false;
    displayManagerServiceProxy_->SetSpecifiedScreenPower(screenId, static_cast<uint32_t>(state),
        static_cast<uint32_t>(reason), isSucc);
    return isSucc;
}

bool ScreenManagerAdapter::SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetScreenPowerForAll(state, reason);
    }

    bool isSucc = false;
    displayManagerServiceProxy_->SetScreenPowerForAll(static_cast<uint32_t>(state),
        static_cast<uint32_t>(reason), isSucc);
    return isSucc;
}

ScreenPowerState ScreenManagerAdapter::GetScreenPower(ScreenId dmsScreenId)
{
    INIT_PROXY_CHECK_RETURN(ScreenPowerState::INVALID_STATE);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetScreenPower(dmsScreenId);
    }

    uint32_t screenPowerState;
    ErrCode errCode = displayManagerServiceProxy_->GetScreenPower(dmsScreenId, screenPowerState);
    if (FAILED(errCode)) {
        TLOGE(WmsLogTag::DMS, "failed, dmsScreenId: %{public}" PRIu64 ", errCode: %{public}d", dmsScreenId, errCode);
        return ScreenPowerState::INVALID_STATE;
    }
    return static_cast<ScreenPowerState>(screenPowerState);
}

ScreenPowerState ScreenManagerAdapter::GetScreenPower()
{
    INIT_PROXY_CHECK_RETURN(ScreenPowerState::INVALID_STATE);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetScreenPower();
    }

    return ScreenPowerState::INVALID_STATE;
}

DMError ScreenManagerAdapter::SetOrientation(ScreenId screenId, Orientation orientation, bool isFromNapi)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetOrientation(screenId, orientation, isFromNapi);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->SetOrientation(screenId, static_cast<uint32_t>(orientation),
        dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError BaseAdapter::RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->RegisterDisplayManagerAgent(displayManagerAgent, type);
    }

    if (type == DisplayManagerAgentType::BRIGHTNESS_INFO_CHANGED_LISTENER) {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->RegisterDisplayManagerAgent(displayManagerAgent,
        static_cast<uint32_t>(type), dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError BaseAdapter::UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->UnregisterDisplayManagerAgent(displayManagerAgent, type);
    }

    if (type == DisplayManagerAgentType::BRIGHTNESS_INFO_CHANGED_LISTENER) {
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->UnregisterDisplayManagerAgent(displayManagerAgent,
        static_cast<uint32_t>(type), dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError DisplayManagerAdapter::RegisterDisplayAttributeAgent(std::vector<std::string>& attributes,
    const sptr<IDisplayManagerAgent> displayManagerAgent)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->RegisterDisplayAttributeAgent(attributes, displayManagerAgent);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError DisplayManagerAdapter::UnRegisterDisplayAttribute(const std::vector<std::string>& attributes,
    const sptr<IDisplayManagerAgent> displayManagerAgent)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->UnRegisterDisplayAttribute(attributes, displayManagerAgent);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

bool DisplayManagerAdapter::WakeUpBegin(PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->WakeUpBegin(reason);
    }

    bool isSucc = false;
    displayManagerServiceProxy_->WakeUpBegin(static_cast<uint32_t>(reason), isSucc);
    return isSucc;
}

bool DisplayManagerAdapter::WakeUpEnd()
{
    INIT_PROXY_CHECK_RETURN(false);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->WakeUpEnd();
    }

    bool isSucc = false;
    displayManagerServiceProxy_->WakeUpEnd(isSucc);
    return isSucc;
}

bool DisplayManagerAdapter::SuspendBegin(PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SuspendBegin(reason);
    }

    bool isSucc = false;
    displayManagerServiceProxy_->SuspendBegin(static_cast<uint32_t>(reason), isSucc);
    return isSucc;
}

bool DisplayManagerAdapter::SuspendEnd()
{
    INIT_PROXY_CHECK_RETURN(false);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SuspendEnd();
    }

    bool isSucc = false;
    displayManagerServiceProxy_->SuspendEnd(isSucc);
    return isSucc;
}

ScreenId DisplayManagerAdapter::GetInternalScreenId()
{
    INIT_PROXY_CHECK_RETURN(false);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetInternalScreenId();
    }

    return SCREEN_ID_INVALID;
}

bool DisplayManagerAdapter::SetScreenPowerById(ScreenId screenId, ScreenPowerState state,
    PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetScreenPowerById(screenId, state, reason);
    }

    return false;
}

bool DisplayManagerAdapter::SetDisplayState(DisplayState state)
{
    INIT_PROXY_CHECK_RETURN(false);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetDisplayState(state);
    }

    bool isSucc = false;
    displayManagerServiceProxy_->SetDisplayState(static_cast<uint32_t>(state), isSucc);
    return isSucc;
}

DisplayState DisplayManagerAdapter::GetDisplayState(DisplayId displayId)
{
    INIT_PROXY_CHECK_RETURN(DisplayState::UNKNOWN);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetDisplayState(displayId);
    }

    uint32_t displayState;
    ErrCode errCode = displayManagerServiceProxy_->GetDisplayState(displayId, displayState);
    if (FAILED(errCode)) {
        TLOGE(WmsLogTag::DMS, "failed, displayId: %{public}" PRIu64 ", errCode: %{public}d", displayId, errCode);
        return DisplayState::UNKNOWN;
    }
    return static_cast<DisplayState>(displayState);
}

bool DisplayManagerAdapter::TryToCancelScreenOff()
{
    INIT_PROXY_CHECK_RETURN(false);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->TryToCancelScreenOff();
    }

    bool isSucc = false;
    displayManagerServiceProxy_->TryToCancelScreenOff(isSucc);
    return isSucc;
}

void DisplayManagerAdapter::NotifyDisplayEvent(DisplayEvent event)
{
    INIT_PROXY_CHECK_RETURN();

    if (screenSessionManagerServiceProxy_) {
        screenSessionManagerServiceProxy_->NotifyDisplayEvent(event);
    } else {
        displayManagerServiceProxy_->NotifyDisplayEvent(static_cast<uint32_t>(event));
    }
}

bool DisplayManagerAdapter::SetFreeze(std::vector<DisplayId> displayIds, bool isFreeze)
{
    INIT_PROXY_CHECK_RETURN(false);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetFreeze(displayIds, isFreeze);
    }

    bool isSucc = false;
    displayManagerServiceProxy_->SetFreeze(displayIds, isFreeze, isSucc);
    return isSucc;
}

bool BaseAdapter::InitDMSProxy()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (isProxyValid_) {
        return true;
    }

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        TLOGE(WmsLogTag::DMS, "Failed to get system ability mgr.");
        return false;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(DISPLAY_MANAGER_SERVICE_SA_ID);
    if (!remoteObject) {
        TLOGE(WmsLogTag::DMS, "Failed to get display manager service.");
        return false;
    }

    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        screenSessionManagerServiceProxy_ = iface_cast<IScreenSessionManager>(remoteObject);
        if ((!screenSessionManagerServiceProxy_) || (!screenSessionManagerServiceProxy_->AsObject())) {
            TLOGE(WmsLogTag::DMS, "Failed to get remote object of IScreenSessionManager");
            return false;
        }
    } else {
        displayManagerServiceProxy_ = iface_cast<IDisplayManager>(remoteObject);
        if ((!displayManagerServiceProxy_) || (!displayManagerServiceProxy_->AsObject())) {
            TLOGE(WmsLogTag::DMS, "Failed to get remote object of IDisplayManager");
            return false;
        }
    }

    dmsDeath_ = new (std::nothrow) DMSDeathRecipient(*this);
    if (dmsDeath_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create death Recipient ptr DMSDeathRecipient");
        return false;
    }
    if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(dmsDeath_)) {
        TLOGE(WmsLogTag::DMS, "Failed to add death recipient");
        return false;
    }

    isProxyValid_ = true;
    return true;
}

DMSDeathRecipient::DMSDeathRecipient(BaseAdapter& adapter) : adapter_(adapter)
{
}

void DMSDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    if (wptrDeath == nullptr) {
        TLOGE(WmsLogTag::DMS, "wptrDeath is null");
        return;
    }

    sptr<IRemoteObject> object = wptrDeath.promote();
    if (!object) {
        TLOGE(WmsLogTag::DMS, "object is null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "dms OnRemoteDied");
    adapter_.Clear();
    if (SingletonContainer::IsDestroyed()) {
        TLOGE(WmsLogTag::DMS, "SingletonContainer is destroyed");
        return;
    }
    SingletonContainer::Get<DisplayManager>().OnRemoteDied();
    SingletonContainer::Get<ScreenManager>().OnRemoteDied();
}


BaseAdapter::~BaseAdapter()
{
    TLOGI(WmsLogTag::DMS, "destroy!");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    Clear();
    screenSessionManagerServiceProxy_ = nullptr;
    displayManagerServiceProxy_ = nullptr;
}

void BaseAdapter::Clear()
{
    TLOGD(WmsLogTag::DMS, "Clear!");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if ((screenSessionManagerServiceProxy_ != nullptr) && (screenSessionManagerServiceProxy_->AsObject() != nullptr)) {
        screenSessionManagerServiceProxy_->AsObject()->RemoveDeathRecipient(dmsDeath_);
    }
    if ((displayManagerServiceProxy_ != nullptr) && (displayManagerServiceProxy_->AsObject() != nullptr)) {
        displayManagerServiceProxy_->AsObject()->RemoveDeathRecipient(dmsDeath_);
    }
    isProxyValid_ = false;
}

DMError ScreenManagerAdapter::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId,
    ScreenId& screenGroupId, const RotationOption& rotationOption)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->MakeMirror(mainScreenId, mirrorScreenId, screenGroupId,
            rotationOption);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->MakeMirror(mainScreenId, mirrorScreenId, screenGroupId, dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::MakeMirrorForRecord(const std::vector<ScreenId>& mainScreenIds,
    std::vector<ScreenId>& mirrorScreenIds, ScreenId& screenGroupId)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->MakeMirrorForRecord(mainScreenIds, mirrorScreenIds, screenGroupId);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId,
    DMRect mainScreenRegion, ScreenId& screenGroupId)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->MakeMirror(mainScreenId, mirrorScreenId, mainScreenRegion,
            screenGroupId);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::SetMultiScreenMode(ScreenId mainScreenId, ScreenId secondaryScreenId,
    MultiScreenMode screenMode)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetMultiScreenMode(mainScreenId, secondaryScreenId, screenMode);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::SetMultiScreenRelativePosition(MultiScreenPositionOptions mainScreenOptions,
    MultiScreenPositionOptions secondScreenOption)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetMultiScreenRelativePosition(mainScreenOptions, secondScreenOption);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::StopMirror(const std::vector<ScreenId>& mirrorScreenIds)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->StopMirror(mirrorScreenIds);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->StopMirror(mirrorScreenIds, dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::DisableMirror(bool disableOrNot)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->DisableMirror(disableOrNot);
    }

    return DMError::DM_ERROR_INVALID_PERMISSION;
}

sptr<ScreenInfo> ScreenManagerAdapter::GetScreenInfo(ScreenId screenId)
{
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screen id is invalid");
        return nullptr;
    }
    INIT_PROXY_CHECK_RETURN(nullptr);

    sptr<ScreenInfo> screenInfo;
    if (screenSessionManagerServiceProxy_) {
        screenInfo = screenSessionManagerServiceProxy_->GetScreenInfoById(screenId);
    } else {
        ErrCode errCode = displayManagerServiceProxy_->GetScreenInfoById(screenId, screenInfo);
        if (FAILED(errCode) || screenInfo == nullptr) {
            TLOGE(WmsLogTag::DMS, "failed, screenId: %{public}" PRIu64 ", errCode: %{public}d, screenInfo: %{public}s",
                screenId, errCode, screenInfo == nullptr ? "null" : "not null");
            return nullptr;
        }
    }
    return screenInfo;
}

std::vector<DisplayId> DisplayManagerAdapter::GetAllDisplayIds(int32_t userId)
{
    TLOGD(WmsLogTag::DMS, "enter");
    INIT_PROXY_CHECK_RETURN(std::vector<DisplayId>());

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetAllDisplayIds(userId);
    }

    std::vector<DisplayId> displayIds;
    displayManagerServiceProxy_->GetAllDisplayIds(displayIds);
    return displayIds;
}

DMError DisplayManagerAdapter::HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->HasPrivateWindow(displayId, hasPrivateWindow);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->HasPrivateWindow(displayId, hasPrivateWindow, dmError);
    return ConvertToDMError(errCode, dmError);
}

bool DisplayManagerAdapter::ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId)
{
    INIT_PROXY_CHECK_RETURN(false);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->ConvertScreenIdToRsScreenId(screenId, rsScreenId);
    }

    return false;
}

DMError DisplayManagerAdapter::HasImmersiveWindow(ScreenId screenId, bool& immersive)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->HasImmersiveWindow(screenId, immersive);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

sptr<DisplayInfo> DisplayManagerAdapter::GetDisplayInfo(DisplayId displayId)
{
    TLOGD(WmsLogTag::DMS, "enter, displayId: %{public}" PRIu64, displayId);
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screen id is invalid");
        return nullptr;
    }
    INIT_PROXY_CHECK_RETURN(nullptr);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetDisplayInfoById(displayId);
    }

    sptr<DisplayInfo> displayInfo;
    ErrCode errCode = displayManagerServiceProxy_->GetDisplayInfoById(displayId, displayInfo);
    if (FAILED(errCode) || displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "failed, displayId: %{public}" PRIu64 ", errCode: %{public}d, displayInfo: %{public}s",
            displayId, errCode, displayInfo == nullptr ? "null" : "not null");
        return nullptr;
    }
    return displayInfo;
}

sptr<DisplayInfo> DisplayManagerAdapter::GetVisibleAreaDisplayInfoById(DisplayId displayId)
{
    TLOGD(WmsLogTag::DMS, "enter, displayId: %{public}" PRIu64, displayId);
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "display id is invalid");
        return nullptr;
    }
    INIT_PROXY_CHECK_RETURN(nullptr);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetVisibleAreaDisplayInfoById(displayId);
    }

    sptr<DisplayInfo> displayInfo;
    ErrCode errCode = displayManagerServiceProxy_->GetVisibleAreaDisplayInfoById(displayId, displayInfo);
    if (FAILED(errCode) || displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "failed, displayId: %{public}" PRIu64 ", errCode: %{public}d, displayInfo: %{public}s",
            displayId, errCode, displayInfo == nullptr ? "null" : "not null");
        return nullptr;
    }
    return displayInfo;
}

sptr<CutoutInfo> DisplayManagerAdapter::GetCutoutInfo(DisplayId displayId, int32_t width,
                                                      int32_t height, Rotation rotation)
{
    TLOGD(WmsLogTag::DMS, "enter");
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "display id is invalid");
        return nullptr;
    }
    INIT_PROXY_CHECK_RETURN(nullptr);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetCutoutInfo(displayId, width, height, rotation);
    }

    sptr<CutoutInfo> cutoutInfo;
    ErrCode errCode = displayManagerServiceProxy_->GetCutoutInfo(displayId, cutoutInfo);
    if (FAILED(errCode) || cutoutInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "failed, displayId: %{public}" PRIu64 ", errCode: %{public}d, cutoutInfo: %{public}s",
            displayId, errCode, cutoutInfo == nullptr ? "null" : "not null");
        return nullptr;
    }
    return cutoutInfo;
}

void PushRoundedCorner(std::vector<RoundedCorner>& roundedCorner, int32_t width, int32_t height, int radius)
{
    RoundedCorner topLeftCorner {CornerType::TOP_LEFT, {radius, radius}, radius};
    RoundedCorner topRightCorner {CornerType::TOP_RIGHT, {width - radius, radius}, radius};
    RoundedCorner bottomRightCorner {CornerType::BOTTOM_RIGHT, {width - radius, height - radius}, radius};
    RoundedCorner bottomLeftCorner {CornerType::BOTTOM_LEFT, {radius, height - radius}, radius};
    roundedCorner.push_back(topLeftCorner);
    roundedCorner.push_back(topRightCorner);
    roundedCorner.push_back(bottomRightCorner);
    roundedCorner.push_back(bottomLeftCorner);
}

DMError DisplayManagerAdapter::GetRoundedCorner(std::vector<RoundedCorner>& roundedCorner,
    DisplayId displayId, int32_t width, int32_t height)
{
    TLOGD(WmsLogTag::DMS, "enter");
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        int radius = 0;
        auto ret = screenSessionManagerServiceProxy_->GetRoundedCorner(displayId, radius);
        if ((ret != DMError::DM_OK) || (radius == 0)) {
            return ret;
        }
        PushRoundedCorner(roundedCorner, width, height, radius);
        return DMError::DM_OK;
    }
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError DisplayManagerAdapter::AddSurfaceNodeToDisplay(DisplayId displayId,
    std::shared_ptr<class RSSurfaceNode>& surfaceNode)
{
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "display id is invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->AddSurfaceNodeToDisplay(displayId, surfaceNode);
    }

    std::shared_ptr<DmRsSurfaceNode> dmRsSurfaceNode = std::make_shared<DmRsSurfaceNode>(surfaceNode);
    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->AddSurfaceNodeToDisplay(displayId, dmRsSurfaceNode, dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError DisplayManagerAdapter::RemoveSurfaceNodeFromDisplay(DisplayId displayId,
    std::shared_ptr<class RSSurfaceNode>& surfaceNode)
{
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "display id is invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->RemoveSurfaceNodeFromDisplay(displayId, surfaceNode);
    }

    int32_t dmError;
    std::shared_ptr<DmRsSurfaceNode> dmRsSurfaceNode = std::make_shared<DmRsSurfaceNode>(surfaceNode);
    ErrCode errCode = displayManagerServiceProxy_->RemoveSurfaceNodeFromDisplay(displayId, dmRsSurfaceNode,
        dmError);
    return ConvertToDMError(errCode, dmError);
}

bool DisplayManagerAdapter::IsFoldable()
{
    INIT_PROXY_CHECK_RETURN(false);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->IsFoldable();
    }

    return false;
}

bool DisplayManagerAdapter::IsCaptured()
{
    INIT_PROXY_CHECK_RETURN(false);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->IsCaptured();
    }

    return false;
}

FoldStatus DisplayManagerAdapter::GetFoldStatus()
{
    INIT_PROXY_CHECK_RETURN(FoldStatus::UNKNOWN);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetFoldStatus();
    }

    return FoldStatus::UNKNOWN;
}

FoldDisplayMode DisplayManagerAdapter::GetFoldDisplayMode()
{
    INIT_PROXY_CHECK_RETURN(FoldDisplayMode::UNKNOWN);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetFoldDisplayMode();
    }

    return FoldDisplayMode::UNKNOWN;
}

void DisplayManagerAdapter::SetFoldDisplayMode(const FoldDisplayMode mode)
{
    INIT_PROXY_CHECK_RETURN();

    if (screenSessionManagerServiceProxy_) {
        screenSessionManagerServiceProxy_->SetFoldDisplayMode(mode);
    }
}

void DisplayManagerAdapter::SetFoldDisplayModeAsync(const FoldDisplayMode mode)
{
    INIT_PROXY_CHECK_RETURN();

    if (screenSessionManagerServiceProxy_) {
        screenSessionManagerServiceProxy_->SetFoldDisplayModeAsync(mode);
    }
}

DMError DisplayManagerAdapter::SetFoldDisplayModeFromJs(const FoldDisplayMode mode, std::string reason)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetFoldDisplayModeFromJs(mode, reason);
    }

    return DMError::DM_OK;
}

void DisplayManagerAdapter::SetDisplayScale(ScreenId screenId,
    float scaleX, float scaleY, float pivotX, float pivotY)
{
    INIT_PROXY_CHECK_RETURN();

    if (screenSessionManagerServiceProxy_) {
        screenSessionManagerServiceProxy_->SetDisplayScale(screenId, scaleX, scaleY, pivotX, pivotY);
    }
}

void DisplayManagerAdapter::SetFoldStatusLocked(bool locked)
{
    INIT_PROXY_CHECK_RETURN();

    if (screenSessionManagerServiceProxy_) {
        screenSessionManagerServiceProxy_->SetFoldStatusLocked(locked);
    }
}

DMError DisplayManagerAdapter::SetFoldStatusLockedFromJs(bool locked)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetFoldStatusLockedFromJs(locked);
    }

    return DMError::DM_OK;
}

DMError DisplayManagerAdapter::ForceSetFoldStatusAndLock(FoldStatus targetFoldStatus)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->ForceSetFoldStatusAndLock(targetFoldStatus);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError DisplayManagerAdapter::RestorePhysicalFoldStatus()
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->RestorePhysicalFoldStatus();
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

sptr<FoldCreaseRegion> DisplayManagerAdapter::GetCurrentFoldCreaseRegion()
{
    INIT_PROXY_CHECK_RETURN(nullptr);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetCurrentFoldCreaseRegion();
    }

    return nullptr;
}

DMError DisplayManagerAdapter::GetLiveCreaseRegion(FoldCreaseRegion& region)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    TLOGI(WmsLogTag::DMS, "enter");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetLiveCreaseRegion(region);
    }
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

sptr<ScreenGroupInfo> ScreenManagerAdapter::GetScreenGroupInfoById(ScreenId screenId)
{
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenGroup id is invalid");
        return nullptr;
    }
    INIT_PROXY_CHECK_RETURN(nullptr);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetScreenGroupInfoById(screenId);
    }

    sptr<ScreenGroupInfo> screenGroupInfo;
    ErrCode errCode = displayManagerServiceProxy_->GetScreenGroupInfoById(screenId, screenGroupInfo);
    if (FAILED(errCode) || screenGroupInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "failed, screenId: %{public}" PRIu64 ", errCode: %{public}d, screenGroupInfo: %{public}s",
            screenId, errCode, screenGroupInfo == nullptr ? "null" : "not null");
        return nullptr;
    }
    return screenGroupInfo;
}

DMError ScreenManagerAdapter::GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetAllScreenInfos(screenInfos);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->GetAllScreenInfos(screenInfos, dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::MakeExpand(std::vector<ScreenId> screenId, std::vector<Point> startPoint,
    ScreenId& screenGroupId)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->MakeExpand(screenId, startPoint, screenGroupId);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->MakeExpand(screenId, startPoint, screenGroupId, dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::StopExpand(const std::vector<ScreenId>& expandScreenIds)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->StopExpand(expandScreenIds);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->StopExpand(expandScreenIds, dmError);
    return ConvertToDMError(errCode, dmError);
}


void ScreenManagerAdapter::RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens)
{
    INIT_PROXY_CHECK_RETURN();

    if (screenSessionManagerServiceProxy_) {
        screenSessionManagerServiceProxy_->RemoveVirtualScreenFromGroup(screens);
    } else {
        displayManagerServiceProxy_->RemoveVirtualScreenFromGroup(screens);
    }
}

DMError ScreenManagerAdapter::SetScreenActiveMode(ScreenId screenId, uint32_t modeId)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetScreenActiveMode(screenId, modeId);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->SetScreenActiveMode(screenId, modeId, dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetVirtualPixelRatio(screenId, virtualPixelRatio);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->SetVirtualPixelRatio(screenId, virtualPixelRatio, dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetVirtualPixelRatioSystem(screenId, virtualPixelRatio);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::SetDefaultDensityDpi(ScreenId screenId, float virtualPixelRatio)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetDefaultDensityDpi(screenId, virtualPixelRatio);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::SetResolution(ScreenId screenId, uint32_t width, uint32_t height, float virtualPixelRatio)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetResolution(screenId, width, height, virtualPixelRatio);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->SetResolution(screenId, width, height, virtualPixelRatio, dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::GetDensityInCurResolution(ScreenId screenId, float& virtualPixelRatio)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetDensityInCurResolution(screenId, virtualPixelRatio);
    }

    int32_t dmError;
    ErrCode errCode = displayManagerServiceProxy_->GetDensityInCurResolution(screenId, virtualPixelRatio, dmError);
    return ConvertToDMError(errCode, dmError);
}

DMError ScreenManagerAdapter::ResizeVirtualScreen(ScreenId screenId, uint32_t width, uint32_t height)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->ResizeVirtualScreen(screenId, width, height);
    }

    return DMError::DM_OK;
}

DMError ScreenManagerAdapter::MakeUniqueScreen(const std::vector<ScreenId>& screenIds,
    std::vector<DisplayId>& displayIds, const UniqueScreenRotationOptions& rotationOptions)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->MakeUniqueScreen(screenIds, displayIds, rotationOptions);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError DisplayManagerAdapter::GetAvailableArea(DisplayId displayId, DMRect& area)
{
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "display id is invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetAvailableArea(displayId, area);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError DisplayManagerAdapter::GetExpandAvailableArea(DisplayId displayId, DMRect& area)
{
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "display id is invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetExpandAvailableArea(displayId, area);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

VirtualScreenFlag ScreenManagerAdapter::GetVirtualScreenFlag(ScreenId screenId)
{
    INIT_PROXY_CHECK_RETURN(VirtualScreenFlag::DEFAULT);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return VirtualScreenFlag::DEFAULT;
    }

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetVirtualScreenFlag(screenId);
    }

    return VirtualScreenFlag::DEFAULT;
}

DMError ScreenManagerAdapter::SetVirtualScreenFlag(ScreenId screenId, VirtualScreenFlag screenFlag)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screen id is invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (screenFlag < VirtualScreenFlag::DEFAULT || screenFlag >= VirtualScreenFlag::MAX) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    TLOGI(WmsLogTag::DMS, "VirtualScreenFlag:%{public}d", screenFlag);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetVirtualScreenFlag(screenId, screenFlag);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::SetVirtualScreenRefreshRate(ScreenId screenId, uint32_t refreshInterval)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetVirtualScreenRefreshRate(screenId, refreshInterval);
    }

    return DMError::DM_OK;
}

DMError DisplayManagerAdapter::ProxyForFreeze(const std::set<int32_t>& pidList, bool isProxy)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->ProxyForFreeze(pidList, isProxy);
    }

    return DMError::DM_OK;
}

DMError DisplayManagerAdapter::ResetAllFreezeStatus()
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->ResetAllFreezeStatus();
    }

    return DMError::DM_OK;
}

void DisplayManagerAdapter::SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList,
    std::vector<uint64_t> surfaceIdList, std::vector<uint8_t> typeBlackList)
{
    INIT_PROXY_CHECK_RETURN();
    if (screenSessionManagerServiceProxy_) {
        screenSessionManagerServiceProxy_->SetVirtualScreenBlackList(screenId, windowIdList, surfaceIdList,
            typeBlackList);
    }
}

DMError DisplayManagerAdapter::IsOnboardDisplay(DisplayId displayId, bool& isOnboardDisplay)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->IsOnboardDisplay(displayId, isOnboardDisplay);
    }
    TLOGE(WmsLogTag::DMS, "fail");
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

void DisplayManagerAdapter::SetVirtualDisplayMuteFlag(ScreenId screenId, bool muteFlag)
{
    INIT_PROXY_CHECK_RETURN();
    if (screenSessionManagerServiceProxy_) {
        screenSessionManagerServiceProxy_->SetVirtualDisplayMuteFlag(screenId, muteFlag);
    }
}

void DisplayManagerAdapter::DisablePowerOffRenderControl(ScreenId screenId)
{
    INIT_PROXY_CHECK_RETURN();
    if (screenSessionManagerServiceProxy_) {
        screenSessionManagerServiceProxy_->DisablePowerOffRenderControl(screenId);
    }
}

std::vector<DisplayPhysicalResolution> DisplayManagerAdapter::GetAllDisplayPhysicalResolution()
{
    INIT_PROXY_CHECK_RETURN(std::vector<DisplayPhysicalResolution>{});
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetAllDisplayPhysicalResolution();
    }

    std::vector<DisplayPhysicalResolution> displayPhysicalResolutions;
    displayManagerServiceProxy_->GetAllDisplayPhysicalResolution(displayPhysicalResolutions);
    return displayPhysicalResolutions;
}

DMError DisplayManagerAdapter::GetDisplayCapability(std::string& capabilitInfo)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetDisplayCapability(capabilitInfo);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

bool ScreenManagerAdapter::SetVirtualScreenStatus(ScreenId screenId, VirtualScreenStatus screenStatus)
{
    INIT_PROXY_CHECK_RETURN(false);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetVirtualScreenStatus(screenId, screenStatus);
    }

    return false;
}

DMError DisplayManagerAdapter::SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid,
    std::vector<uint64_t>& windowIdList)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetVirtualScreenSecurityExemption(screenId, pid, windowIdList);
    }

    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::SetVirtualScreenMaxRefreshRate(ScreenId id, uint32_t refreshRate,
    uint32_t& actualRefreshRate)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetVirtualScreenMaxRefreshRate(id, refreshRate, actualRefreshRate);
    }

    return DMError::DM_OK;
}

std::shared_ptr<Media::PixelMap> DisplayManagerAdapter::GetScreenCapture(const CaptureOption& captureOption,
    DmErrorCode* errorCode)
{
    INIT_PROXY_CHECK_RETURN(nullptr);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetScreenCapture(captureOption, errorCode);
    }

    if (errorCode != nullptr) {
        *errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    return nullptr;
}

sptr<DisplayInfo> DisplayManagerAdapter::GetPrimaryDisplayInfo()
{
    INIT_PROXY_CHECK_RETURN(nullptr);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetPrimaryDisplayInfo();
    }

    sptr<DisplayInfo> displayInfo;
    ErrCode errCode = displayManagerServiceProxy_->GetDefaultDisplayInfo(displayInfo);
    if (FAILED(errCode) || displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "failed, errCode: %{public}d, displayInfo: %{public}s",
            errCode, displayInfo == nullptr ? "null" : "not null");
        return nullptr;
    }
    return displayInfo;
}

DisplayId DisplayManagerAdapter::GetPrimaryDisplayId()
{
    INIT_PROXY_CHECK_RETURN(SCREEN_ID_INVALID);
    DisplayId id = SCREEN_ID_INVALID;
    if (screenSessionManagerServiceProxy_) {
        id = screenSessionManagerServiceProxy_->GetPrimaryDisplayId();
    }
    return id;
}

std::shared_ptr<Media::PixelMap> DisplayManagerAdapter::GetDisplaySnapshotWithOption(const CaptureOption& captureOption,
    DmErrorCode* errorCode)
{
    INIT_PROXY_CHECK_RETURN(nullptr);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetDisplaySnapshotWithOption(captureOption, errorCode);
    }

    if (errorCode != nullptr) {
        *errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Media::PixelMap>> DisplayManagerAdapter::GetDisplayHDRSnapshotWithOption(
    const CaptureOption& captureOption, DmErrorCode& errorCode)
{
    std::vector<std::shared_ptr<Media::PixelMap>> ret = { nullptr, nullptr };
    INIT_PROXY_CHECK_RETURN(ret);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetDisplayHDRSnapshotWithOption(captureOption, errorCode);
    }
    errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
    return ret;
}

void ScreenManagerAdapter::SetFoldStatusExpandAndLocked(bool locked)
{
    INIT_PROXY_CHECK_RETURN();
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetFoldStatusExpandAndLocked(locked);
    }
}

DMError ScreenManagerAdapter::SetScreenSkipProtectedWindow(const std::vector<ScreenId>& screenIds, bool isEnable)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetScreenSkipProtectedWindow(screenIds, isEnable);
    }

    return DMError::DM_OK;
}

DMError DisplayManagerAdapter::GetScreenAreaOfDisplayArea(DisplayId displayId, const DMRect& displayArea,
    ScreenId& screenId, DMRect& screenArea)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetScreenAreaOfDisplayArea(displayId, displayArea, screenId,
            screenArea);
    }
    return DMError::DM_OK;
}

DMError DisplayManagerAdapter::GetBrightnessInfo(DisplayId displayId, ScreenBrightnessInfo& brightnessInfo)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetBrightnessInfo(displayId, brightnessInfo);
    }
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError DisplayManagerAdapter::GetSupportsInput(DisplayId displayId, bool& supportsInput)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetSupportsInput(displayId, supportsInput);
    }
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError DisplayManagerAdapter::SetSupportsInput(DisplayId displayId, bool supportsInput)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetSupportsInput(displayId, supportsInput);
    }
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError DisplayManagerAdapter::GetBundleName(DisplayId displayId, std::string& bundleName)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->GetBundleName(displayId, bundleName);
    }
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

DMError ScreenManagerAdapter::SetVirtualScreenAutoRotation(ScreenId screenId, bool enable)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    TLOGI(WmsLogTag::DMS, "enter!");
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetVirtualScreenAutoRotation(screenId, enable);
    }
    return DMError::DM_OK;
}

DMError ScreenManagerAdapter::SetScreenPrivacyWindowTagSwitch(ScreenId screenId,
    const std::vector<std::string>& privacyWindowTag, bool enable)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    if (screenSessionManagerServiceProxy_) {
        return screenSessionManagerServiceProxy_->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTag, enable);
    }
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}
} // namespace OHOS::Rosen
