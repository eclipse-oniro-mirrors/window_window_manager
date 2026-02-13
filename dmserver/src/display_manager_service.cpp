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

#include "display_manager_service.h"

#include <hitrace_meter.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "display_manager_agent_controller.h"
#include "display_manager_config.h"
#include "dm_common.h"
#include "parameters.h"
#include "permission.h"
#include "sensor_connector.h"
#include "transaction/rs_interfaces.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
const std::string SCREEN_CAPTURE_PERMISSION = "ohos.permission.CAPTURE_SCREEN";
const std::string CUSTOM_SCREEN_RECORDING_PERMISSION = "ohos.permission.CUSTOM_SCREEN_RECORDING";
const std::string ACCESS_VIRTUAL_SCREEN_PERMISSION = "ohos.permission.ACCESS_VIRTUAL_SCREEN";
}
WM_IMPLEMENT_SINGLE_INSTANCE(DisplayManagerService)

#define CHECK_SCREEN_AND_RETURN(screenId, ret) \
    do { \
        if ((screenId) == SCREEN_ID_INVALID) { \
            TLOGE(WmsLogTag::DMS, "screenId invalid"); \
            return ret; \
        } \
    } while (false)

DisplayManagerService::DisplayManagerService()
    : abstractDisplayController_(new AbstractDisplayController(mutex_,
        std::bind(&DisplayManagerService::NotifyDisplayStateChange, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4))),
    abstractScreenController_(new AbstractScreenController(mutex_)),
    displayPowerController_(new DisplayPowerController(mutex_,
        std::bind(&DisplayManagerService::NotifyDisplayStateChange, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4))),
    displayCutoutController_(new DisplayCutoutController()),
    isAutoRotationOpen_(OHOS::system::GetParameter(
        "persist.display.ar.enabled", "1") == "1") // autoRotation default enabled
{
}

int DisplayManagerService::Dump(int fd, const std::vector<std::u16string>& args)
{
    if (displayDumper_ == nullptr) {
        displayDumper_ = new DisplayDumper(abstractDisplayController_, abstractScreenController_, mutex_);
    }
    return static_cast<int>(displayDumper_->Dump(fd, args));
}

bool DisplayManagerService::Init()
{
    TLOGI(WmsLogTag::DMS, "start");
    if (DisplayManagerConfig::LoadConfigXml()) {
        DisplayManagerConfig::DumpConfig();
        ConfigureDisplayManagerService();
    }
    abstractScreenController_->Init();
    abstractDisplayController_->Init(abstractScreenController_);
    TLOGI(WmsLogTag::DMS, "success");
    return true;
}

void DisplayManagerService::ConfigureDisplayManagerService()
{
    auto numbersConfig = DisplayManagerConfig::GetIntNumbersConfig();
    auto enableConfig = DisplayManagerConfig::GetEnableConfig();
    auto stringConfig = DisplayManagerConfig::GetStringConfig();
    if (numbersConfig.count("defaultDeviceRotationOffset") != 0) {
        uint32_t defaultDeviceRotationOffset = static_cast<uint32_t>(numbersConfig["defaultDeviceRotationOffset"][0]);
        ScreenRotationController::SetDefaultDeviceRotationOffset(defaultDeviceRotationOffset);
    }
    if (enableConfig.count("isWaterfallDisplay") != 0) {
        displayCutoutController_->SetIsWaterfallDisplay(
            static_cast<bool>(enableConfig["isWaterfallDisplay"]));
    }
    if (numbersConfig.count("curvedScreenBoundary") != 0) {
        displayCutoutController_->SetCurvedScreenBoundary(
            static_cast<std::vector<int>>(numbersConfig["curvedScreenBoundary"]));
    }
    if (stringConfig.count("defaultDisplayCutoutPath") != 0) {
        displayCutoutController_->SetBuiltInDisplayCutoutSvgPath(
            static_cast<std::string>(stringConfig["defaultDisplayCutoutPath"]));
    }
    ConfigureWaterfallDisplayCompressionParams();
    if (numbersConfig.count("buildInDefaultOrientation") != 0) {
        Orientation orientation = static_cast<Orientation>(numbersConfig["buildInDefaultOrientation"][0]);
        abstractScreenController_->SetBuildInDefaultOrientation(orientation);
    }
}

void DisplayManagerService::ConfigureWaterfallDisplayCompressionParams()
{
    auto numbersConfig = DisplayManagerConfig::GetIntNumbersConfig();
    auto enableConfig = DisplayManagerConfig::GetEnableConfig();
    if (enableConfig.count("isWaterfallAreaCompressionEnableWhenHorizontal") != 0) {
        DisplayCutoutController::SetWaterfallAreaCompressionEnableWhenHorzontal(
            static_cast<bool>(enableConfig["isWaterfallAreaCompressionEnableWhenHorizontal"]));
    }
    if (numbersConfig.count("waterfallAreaCompressionSizeWhenHorzontal") != 0) {
        DisplayCutoutController::SetWaterfallAreaCompressionSizeWhenHorizontal(
            static_cast<uint32_t>(numbersConfig["waterfallAreaCompressionSizeWhenHorzontal"][0]));
    }
}

void DisplayManagerService::RegisterDisplayChangeListener(sptr<IDisplayChangeListener> listener)
{
    displayChangeListener_ = listener;
    TLOGD(WmsLogTag::DMS, "IDisplayChangeListener registered");
}

void DisplayManagerService::RegisterWindowInfoQueriedListener(const sptr<IWindowInfoQueriedListener>& listener)
{
    windowInfoQueriedListener_ = listener;
}

DMError DisplayManagerService::HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "check has private window permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    std::vector<DisplayId> displayIds = GetAllDisplayIds();
    auto iter = std::find(displayIds.begin(), displayIds.end(), displayId);
    if (iter == displayIds.end()) {
        TLOGE(WmsLogTag::DMS, "invalid displayId");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (windowInfoQueriedListener_ != nullptr) {
        windowInfoQueriedListener_->HasPrivateWindow(displayId, hasPrivateWindow);
        return DMError::DM_OK;
    }
    return DMError::DM_ERROR_NULLPTR;
}

void DisplayManagerService::NotifyDisplayStateChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type)
{
    DisplayId id = (displayInfo == nullptr) ? DISPLAY_ID_INVALID : displayInfo->GetDisplayId();
    TLOGD(WmsLogTag::DMS, "DisplayId %{public}" PRIu64, id);
    if (displayChangeListener_ != nullptr) {
        displayChangeListener_->OnDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    }
}

void DisplayManagerService::NotifyScreenshot(DisplayId displayId)
{
    if (displayChangeListener_ != nullptr) {
        displayChangeListener_->OnScreenshot(displayId);
    }
}

sptr<DisplayInfo> DisplayManagerService::GetDefaultDisplayInfo()
{
    ScreenId dmsScreenId = abstractScreenController_->GetDefaultAbstractScreenId();
    TLOGD(WmsLogTag::DMS, "GetDefaultDisplayInfo %{public}" PRIu64, dmsScreenId);
    sptr<AbstractDisplay> display = abstractDisplayController_->GetAbstractDisplayByScreen(dmsScreenId);
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail to get displayInfo by id: invalid display");
        return nullptr;
    }
    return display->ConvertToDisplayInfo();
}

sptr<DisplayInfo> DisplayManagerService::GetDisplayInfoById(DisplayId displayId)
{
    sptr<AbstractDisplay> display = abstractDisplayController_->GetAbstractDisplay(displayId);
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail to get displayInfo by id: invalid display");
        return nullptr;
    }
    return display->ConvertToDisplayInfo();
}

sptr<DisplayInfo> DisplayManagerService::GetVisibleAreaDisplayInfoById(DisplayId displayId)
{
    sptr<AbstractDisplay> display = abstractDisplayController_->GetAbstractDisplay(displayId);
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail to get displayInfo by id: invalid display");
        return nullptr;
    }
    return display->ConvertToDisplayInfo();
}

sptr<DisplayInfo> DisplayManagerService::GetDisplayInfoByScreen(ScreenId screenId)
{
    sptr<AbstractDisplay> display = abstractDisplayController_->GetAbstractDisplayByScreen(screenId);
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail to get displayInfo by screenId: invalid display");
        return nullptr;
    }
    return display->ConvertToDisplayInfo();
}

ScreenId DisplayManagerService::CreateVirtualScreen(VirtualScreenOption option,
    const sptr<IRemoteObject>& displayManagerAgent)
{
    if (displayManagerAgent == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayManagerAgent invalid");
        return SCREEN_ID_INVALID;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:CreateVirtualScreen(%s)", option.name_.c_str());
    if (option.surface_ != nullptr && !Permission::CheckCallingPermission(SCREEN_CAPTURE_PERMISSION) &&
        !Permission::IsStartByHdcd() && !Permission::CheckCallingPermission(ACCESS_VIRTUAL_SCREEN_PERMISSION)) {
        TLOGE(WmsLogTag::DMS, "permission denied");
        return SCREEN_ID_INVALID;
    }
    ScreenId screenId = abstractScreenController_->CreateVirtualScreen(option, displayManagerAgent);
    CHECK_SCREEN_AND_RETURN(screenId, SCREEN_ID_INVALID);
    accessTokenIdMaps_.insert(std::pair(screenId, IPCSkeleton::GetCallingTokenID()));
    return screenId;
}

DMError DisplayManagerService::DestroyVirtualScreen(ScreenId screenId)
{
    bool isCallingByThirdParty = Permission::CheckCallingPermission(ACCESS_VIRTUAL_SCREEN_PERMISSION);
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd() &&
        !isCallingByThirdParty) {
        TLOGE(WmsLogTag::DMS, "destroy virtual screen permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (!accessTokenIdMaps_.isExistAndRemove(screenId, IPCSkeleton::GetCallingTokenID())) {
        if (isCallingByThirdParty) {
            return DMError::DM_ERROR_NULLPTR;
        }
        return DMError::DM_ERROR_INVALID_CALLING;
    }

    TLOGI(WmsLogTag::DMS, "DestroyVirtualScreen::ScreenId: %{public}" PRIu64, screenId);
    CHECK_SCREEN_AND_RETURN(screenId, DMError::DM_ERROR_INVALID_PARAM);

    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:DestroyVirtualScreen(%" PRIu64 ")", screenId);
    return abstractScreenController_->DestroyVirtualScreen(screenId);
}

DMError DisplayManagerService::SetVirtualScreenSurface(ScreenId screenId, sptr<IBufferProducer> surface)
{
    TLOGI(WmsLogTag::DMS, "SetVirtualScreenSurface::ScreenId: %{public}" PRIu64, screenId);
    bool isCallingByThirdParty = Permission::CheckCallingPermission(ACCESS_VIRTUAL_SCREEN_PERMISSION);
    CHECK_SCREEN_AND_RETURN(screenId, DMError::DM_ERROR_INVALID_PARAM);
    if (Permission::CheckCallingPermission(SCREEN_CAPTURE_PERMISSION) ||
        Permission::IsStartByHdcd() || isCallingByThirdParty) {
        sptr<Surface> pPurface = Surface::CreateSurfaceAsProducer(surface);
        return abstractScreenController_->SetVirtualScreenSurface(screenId, pPurface);
    }
    TLOGE(WmsLogTag::DMS, "permission denied");
    if (isCallingByThirdParty) {
        return DMError::DM_ERROR_NULLPTR;
    }
    return DMError::DM_ERROR_INVALID_CALLING;
}

DMError DisplayManagerService::SetOrientation(ScreenId screenId, Orientation orientation)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "set orientation permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (orientation < Orientation::UNSPECIFIED || orientation > Orientation::REVERSE_HORIZONTAL) {
        TLOGE(WmsLogTag::DMS, "orientation: %{public}u", static_cast<uint32_t>(orientation));
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:SetOrientation(%" PRIu64 ")", screenId);
    return abstractScreenController_->SetOrientation(screenId, orientation, false);
}

DMError DisplayManagerService::SetOrientationFromWindow(ScreenId screenId, Orientation orientation, bool withAnimation)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:SetOrientationFromWindow(%" PRIu64 ")", screenId);
    return abstractScreenController_->SetOrientation(screenId, orientation, true, withAnimation);
}

bool DisplayManagerService::SetRotationFromWindow(ScreenId screenId, Rotation targetRotation, bool withAnimation)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:SetRotationFromWindow(%" PRIu64 ")", screenId);
    return abstractScreenController_->SetRotation(screenId, targetRotation, true, withAnimation);
}

std::shared_ptr<Media::PixelMap> DisplayManagerService::GetDisplaySnapshot(DisplayId displayId,
    DmErrorCode* errorCode, bool isUseDma, bool isCaptureFullOfScreen)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:GetDisplaySnapshot(%" PRIu64 ")", displayId);
    if ((Permission::IsSystemCalling() && (Permission::CheckCallingPermission(SCREEN_CAPTURE_PERMISSION) ||
        Permission::CheckCallingPermission(CUSTOM_SCREEN_RECORDING_PERMISSION))) || Permission::IsStartByHdcd()) {
        auto res = abstractDisplayController_->GetScreenSnapshot(displayId);
        if (res != nullptr) {
            NotifyScreenshot(displayId);
        }
        return res;
    } else if (errorCode) {
        *errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
    }
    return nullptr;
}

DMError DisplayManagerService::GetScreenSupportedColorGamuts(ScreenId screenId,
    std::vector<ScreenColorGamut>& colorGamuts)
{
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64, screenId);
    CHECK_SCREEN_AND_RETURN(screenId, DMError::DM_ERROR_INVALID_PARAM);
    return abstractScreenController_->GetScreenSupportedColorGamuts(screenId, colorGamuts);
}

DMError DisplayManagerService::GetScreenColorGamut(ScreenId screenId, ScreenColorGamut& colorGamut)
{
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64, screenId);
    CHECK_SCREEN_AND_RETURN(screenId, DMError::DM_ERROR_INVALID_PARAM);
    return abstractScreenController_->GetScreenColorGamut(screenId, colorGamut);
}

DMError DisplayManagerService::SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx)
{
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64 ", colorGamutIdx %{public}d", screenId,
        colorGamutIdx);
    CHECK_SCREEN_AND_RETURN(screenId, DMError::DM_ERROR_INVALID_PARAM);
    return abstractScreenController_->SetScreenColorGamut(screenId, colorGamutIdx);
}

DMError DisplayManagerService::GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap)
{
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64, screenId);
    CHECK_SCREEN_AND_RETURN(screenId, DMError::DM_ERROR_INVALID_PARAM);
    return abstractScreenController_->GetScreenGamutMap(screenId, gamutMap);
}

DMError DisplayManagerService::SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap)
{
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64 ", ScreenGamutMap %{public}u",
        screenId, static_cast<uint32_t>(gamutMap));
    CHECK_SCREEN_AND_RETURN(screenId, DMError::DM_ERROR_INVALID_PARAM);
    return abstractScreenController_->SetScreenGamutMap(screenId, gamutMap);
}

DMError DisplayManagerService::SetScreenColorTransform(ScreenId screenId)
{
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64, screenId);
    CHECK_SCREEN_AND_RETURN(screenId, DMError::DM_ERROR_INVALID_PARAM);
    return abstractScreenController_->SetScreenColorTransform(screenId);
}

DMError DisplayManagerService::RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
{
    if (type == DisplayManagerAgentType::SCREEN_EVENT_LISTENER && !Permission::IsSystemCalling()
        && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "register display manager agent permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if ((displayManagerAgent == nullptr) || (displayManagerAgent->AsObject() == nullptr)) {
        TLOGE(WmsLogTag::DMS, "displayManagerAgent invalid");
        return DMError::DM_ERROR_NULLPTR;
    }
    return DisplayManagerAgentController::GetInstance().RegisterDisplayManagerAgent(displayManagerAgent, type);
}

DMError DisplayManagerService::UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
{
    if (type == DisplayManagerAgentType::SCREEN_EVENT_LISTENER && !Permission::IsSystemCalling()
        && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "unregister display manager agent permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if ((displayManagerAgent == nullptr) || (displayManagerAgent->AsObject() == nullptr)) {
        TLOGE(WmsLogTag::DMS, "displayManagerAgent invalid");
        return DMError::DM_ERROR_NULLPTR;
    }
    return DisplayManagerAgentController::GetInstance().UnregisterDisplayManagerAgent(displayManagerAgent, type);
}

bool DisplayManagerService::WakeUpBegin(PowerStateChangeReason reason)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[UL_POWER]dms:WakeUpBegin(%u)", reason);
    if (!Permission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]wake up begin permission denied!");
        return false;
    }
    return DisplayManagerAgentController::GetInstance().NotifyDisplayPowerEvent(DisplayPowerEvent::WAKE_UP,
        EventStatus::BEGIN);
}

bool DisplayManagerService::WakeUpEnd()
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[UL_POWER]dms:WakeUpEnd");
    if (!Permission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]wake up end permission denied!");
        return false;
    }
    return DisplayManagerAgentController::GetInstance().NotifyDisplayPowerEvent(DisplayPowerEvent::WAKE_UP,
        EventStatus::END);
}

bool DisplayManagerService::SuspendBegin(PowerStateChangeReason reason)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[UL_POWER]dms:SuspendBegin(%u)", reason);
    if (!Permission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]suspend begin permission denied!");
        return false;
    }
    displayPowerController_->SuspendBegin(reason);
    return DisplayManagerAgentController::GetInstance().NotifyDisplayPowerEvent(DisplayPowerEvent::SLEEP,
        EventStatus::BEGIN);
}

bool DisplayManagerService::SuspendEnd()
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[UL_POWER]dms:SuspendEnd");
    if (!Permission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]suspend end permission denied!");
        return false;
    }
    return DisplayManagerAgentController::GetInstance().NotifyDisplayPowerEvent(DisplayPowerEvent::SLEEP,
        EventStatus::END);
}

bool DisplayManagerService::SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state,
    PowerStateChangeReason reason)
{
    TLOGE(WmsLogTag::DMS, "[UL_POWER]DMS not support SetSpecifiedScreenPower: screen:%{public}" PRIu64
        ", "
        "state:%{public}u", screenId, state);
    return false;
}

bool DisplayManagerService::SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason)
{
    TLOGI(WmsLogTag::DMS, "[UL_POWER]SetScreenPowerForAll");
    if (!Permission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]set screen power for all permission denied!");
        return false;
    }
    return abstractScreenController_->SetScreenPowerForAll(state, reason);
}

ScreenPowerState DisplayManagerService::GetScreenPower(ScreenId dmsScreenId)
{
    return abstractScreenController_->GetScreenPower(dmsScreenId);
}

bool DisplayManagerService::SetDisplayState(DisplayState state)
{
    if (!Permission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]set display state permission denied!");
        return false;
    }
    ScreenId dmsScreenId = abstractScreenController_->GetDefaultAbstractScreenId();
    sptr<AbstractDisplay> display = abstractDisplayController_->GetAbstractDisplayByScreen(dmsScreenId);
    if (display != nullptr) {
        display->SetDisplayState(state);
    }
    return displayPowerController_->SetDisplayState(state);
}

ScreenId DisplayManagerService::GetScreenIdByDisplayId(DisplayId displayId) const
{
    sptr<AbstractDisplay> abstractDisplay = abstractDisplayController_->GetAbstractDisplay(displayId);
    if (abstractDisplay == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetAbstractDisplay failed");
        return SCREEN_ID_INVALID;
    }
    return abstractDisplay->GetAbstractScreenId();
}

DisplayState DisplayManagerService::GetDisplayState(DisplayId displayId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return displayPowerController_->GetDisplayState(displayId);
}

bool DisplayManagerService::TryToCancelScreenOff()
{
    TLOGE(WmsLogTag::DMS, "[UL_POWER]DMS not support TryToCancelScreenOff");
    return false;
}

bool DisplayManagerService::SetScreenBrightness(uint64_t screenId, uint32_t level)
{
    if (!Permission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::DMS, "set screen brightness permission denied!");
        return false;
    }
    RSInterfaces::GetInstance().SetScreenBacklight(screenId, level);
    return true;
}

uint32_t DisplayManagerService::GetScreenBrightness(uint64_t screenId)
{
    uint32_t level = static_cast<uint32_t>(RSInterfaces::GetInstance().GetScreenBacklight(screenId));
    TLOGD(WmsLogTag::DMS, "GetScreenBrightness screenId:%{public}" PRIu64 ", level:%{public}u,",
        screenId, level);
    return level;
}

void DisplayManagerService::NotifyDisplayEvent(DisplayEvent event)
{
    if (!Permission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]notify display event permission denied!");
        return;
    }
    displayPowerController_->NotifyDisplayEvent(event);
}

bool DisplayManagerService::SetFreeze(std::vector<DisplayId> displayIds, bool isFreeze)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "set freeze permission denied!");
        return false;
    }
    abstractDisplayController_->SetFreeze(displayIds, isFreeze);
    return true;
}

DMError DisplayManagerService::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
                                          ScreenId& screenGroupId)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "make mirror permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    TLOGI(WmsLogTag::DMS, "MakeMirror. mainScreenId :%{public}" PRIu64, mainScreenId);
    auto allMirrorScreenIds = abstractScreenController_->GetAllValidScreenIds(mirrorScreenIds);
    auto iter = std::find(allMirrorScreenIds.begin(), allMirrorScreenIds.end(), mainScreenId);
    if (iter != allMirrorScreenIds.end()) {
        allMirrorScreenIds.erase(iter);
    }
    auto mainScreen = abstractScreenController_->GetAbstractScreen(mainScreenId);
    if (mainScreen == nullptr || allMirrorScreenIds.empty()) {
        TLOGI(WmsLogTag::DMS, "create mirror fail. main screen :%{public}" PRIu64 ", screens' size:%{public}u",
            mainScreenId, static_cast<uint32_t>(allMirrorScreenIds.size()));
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:MakeMirror");
    DMError ret = abstractScreenController_->MakeMirror(mainScreenId, allMirrorScreenIds);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "make mirror failed.");
        return ret;
    }
    if (abstractScreenController_->GetAbstractScreenGroup(mainScreen->groupDmsId_) == nullptr) {
        TLOGE(WmsLogTag::DMS, "get screen group failed.");
        return DMError::DM_ERROR_NULLPTR;
    }
    screenGroupId = mainScreen->groupDmsId_;
    return DMError::DM_OK;
}

DMError DisplayManagerService::StopMirror(const std::vector<ScreenId>& mirrorScreenIds)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "stop mirror permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }

    auto allMirrorScreenIds = abstractScreenController_->GetAllValidScreenIds(mirrorScreenIds);
    if (allMirrorScreenIds.empty()) {
        TLOGI(WmsLogTag::DMS, "stop mirror done. screens' size:%{public}u",
            static_cast<uint32_t>(allMirrorScreenIds.size()));
        return DMError::DM_OK;
    }

    DMError ret = abstractScreenController_->StopScreens(allMirrorScreenIds, ScreenCombination::SCREEN_MIRROR);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "stop mirror failed.");
        return ret;
    }

    return DMError::DM_OK;
}

void DisplayManagerService::RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens)
{
    abstractScreenController_->RemoveVirtualScreenFromGroup(screens);
}

void DisplayManagerService::UpdateRSTree(DisplayId displayId, DisplayId parentDisplayId,
    std::shared_ptr<RSSurfaceNode>& surfaceNode, bool isAdd, bool isMultiDisplay)
{
    TLOGD(WmsLogTag::DMS, "UpdateRSTree, currentDisplayId: %{public}" PRIu64
        ", isAdd: %{public}d, isMultiDisplay: %{public}d, parentDisplayId: %{public}" PRIu64,
        displayId, isAdd, isMultiDisplay, parentDisplayId);
    ScreenId screenId = GetScreenIdByDisplayId(displayId);
    ScreenId parentScreenId = GetScreenIdByDisplayId(parentDisplayId);
    CHECK_SCREEN_AND_RETURN(screenId, void());

    abstractScreenController_->UpdateRSTree(screenId, parentScreenId, surfaceNode, isAdd, isMultiDisplay);
}

DMError DisplayManagerService::AddSurfaceNodeToDisplay(DisplayId displayId,
    std::shared_ptr<RSSurfaceNode>& surfaceNode, bool onTop)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    TLOGI(WmsLogTag::DMS, "DisplayId: %{public}" PRIu64 ", onTop: %{public}d", displayId, onTop);
    if (surfaceNode == nullptr) {
        TLOGW(WmsLogTag::DMS, "Surface is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    ScreenId screenId = GetScreenIdByDisplayId(displayId);
    return abstractScreenController_->AddSurfaceNodeToScreen(screenId, surfaceNode, true);
}

DMError DisplayManagerService::RemoveSurfaceNodeFromDisplay(DisplayId displayId,
    std::shared_ptr<RSSurfaceNode>& surfaceNode)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    TLOGI(WmsLogTag::DMS, "DisplayId: %{public}" PRIu64, displayId);
    if (surfaceNode == nullptr) {
        TLOGW(WmsLogTag::DMS, "Surface is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    ScreenId screenId = GetScreenIdByDisplayId(displayId);
    return abstractScreenController_->RemoveSurfaceNodeFromScreen(screenId, surfaceNode);
}

sptr<ScreenInfo> DisplayManagerService::GetScreenInfoById(ScreenId screenId)
{
    auto screen = abstractScreenController_->GetAbstractScreen(screenId);
    if (screen == nullptr) {
        TLOGE(WmsLogTag::DMS, "cannot find screenInfo: %{public}" PRIu64, screenId);
        return nullptr;
    }
    return screen->ConvertToScreenInfo();
}

sptr<ScreenGroupInfo> DisplayManagerService::GetScreenGroupInfoById(ScreenId screenId)
{
    auto screenGroup = abstractScreenController_->GetAbstractScreenGroup(screenId);
    if (screenGroup == nullptr) {
        TLOGE(WmsLogTag::DMS, "cannot find screenGroupInfo: %{public}" PRIu64, screenId);
        return nullptr;
    }
    return screenGroup->ConvertToScreenGroupInfo();
}

ScreenId DisplayManagerService::GetScreenGroupIdByScreenId(ScreenId screenId)
{
    auto screen = abstractScreenController_->GetAbstractScreen(screenId);
    if (screen == nullptr) {
        TLOGE(WmsLogTag::DMS, "cannot find screenInfo: %{public}" PRIu64, screenId);
        return SCREEN_ID_INVALID;
    }
    return screen->GetScreenGroupId();
}

std::vector<DisplayId> DisplayManagerService::GetAllDisplayIds()
{
    return abstractDisplayController_->GetAllDisplayIds();
}

DMError DisplayManagerService::GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "get all screen infos permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    std::vector<ScreenId> screenIds = abstractScreenController_->GetAllScreenIds();
    for (auto screenId: screenIds) {
        auto screenInfo = GetScreenInfoById(screenId);
        if (screenInfo == nullptr) {
            TLOGE(WmsLogTag::DMS, "cannot find screenInfo: %{public}" PRIu64, screenId);
            continue;
        }
        screenInfos.emplace_back(screenInfo);
    }
    return DMError::DM_OK;
}

DMError DisplayManagerService::MakeExpand(std::vector<ScreenId> expandScreenIds, std::vector<Point> startPoints,
    ScreenId& screenGroupId)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "make expand permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (expandScreenIds.empty() || startPoints.empty() || expandScreenIds.size() != startPoints.size()) {
        TLOGE(WmsLogTag::DMS, "create expand fail, input params is invalid. "
            "screenId vector size :%{public}u, startPoint vector size :%{public}u",
            static_cast<uint32_t>(expandScreenIds.size()), static_cast<uint32_t>(startPoints.size()));
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    std::map<ScreenId, Point> pointsMap;
    uint32_t size = expandScreenIds.size();
    for (uint32_t i = 0; i < size; i++) {
        if (pointsMap.find(expandScreenIds[i]) != pointsMap.end()) {
            continue;
        }
        pointsMap[expandScreenIds[i]] = startPoints[i];
    }
    ScreenId defaultScreenId = abstractScreenController_->GetDefaultAbstractScreenId();
    TLOGI(WmsLogTag::DMS, "MakeExpand, defaultScreenId:%{public}" PRIu64, defaultScreenId);
    auto allExpandScreenIds = abstractScreenController_->GetAllValidScreenIds(expandScreenIds);
    auto iter = std::find(allExpandScreenIds.begin(), allExpandScreenIds.end(), defaultScreenId);
    if (iter != allExpandScreenIds.end()) {
        allExpandScreenIds.erase(iter);
    }
    if (allExpandScreenIds.empty()) {
        TLOGE(WmsLogTag::DMS, "allExpandScreenIds is empty. make expand failed.");
        return DMError::DM_ERROR_NULLPTR;
    }
    std::shared_ptr<RSDisplayNode> rsDisplayNode;
    std::vector<Point> points;
    for (uint32_t i = 0; i < allExpandScreenIds.size(); i++) {
        ScreenId expandScreenId = allExpandScreenIds[i];
        if (pointsMap.find(expandScreenId) == pointsMap.end()) {
            continue;
        }
        points.emplace_back(pointsMap[expandScreenId]);
        rsDisplayNode = abstractScreenController_->GetRSDisplayNodeByScreenId(expandScreenId);
        if (rsDisplayNode != nullptr) {
            sptr<AbstractScreen> absScreen = abstractScreenController_->GetAbstractScreen(expandScreenId);
            if (absScreen != nullptr) {
                RSInterfaces::GetInstance().SetScreenOffset(absScreen->rsId_,
                    pointsMap[expandScreenId].posX_, pointsMap[expandScreenId].posY_);
            }
        }
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:MakeExpand");
    if (!abstractScreenController_->MakeExpand(allExpandScreenIds, points)) {
        TLOGE(WmsLogTag::DMS, "make expand failed.");
        return DMError::DM_ERROR_NULLPTR;
    }
    auto screen = abstractScreenController_->GetAbstractScreen(allExpandScreenIds[0]);
    if (screen == nullptr || abstractScreenController_->GetAbstractScreenGroup(screen->groupDmsId_) == nullptr) {
        TLOGE(WmsLogTag::DMS, "get screen group failed.");
        return DMError::DM_ERROR_NULLPTR;
    }
    screenGroupId = screen->groupDmsId_;
    return DMError::DM_OK;
}

DMError DisplayManagerService::StopExpand(const std::vector<ScreenId>& expandScreenIds)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "stop expand permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    auto allExpandScreenIds = abstractScreenController_->GetAllValidScreenIds(expandScreenIds);
    if (allExpandScreenIds.empty()) {
        TLOGI(WmsLogTag::DMS, "stop expand done. screens' size:%{public}u",
            static_cast<uint32_t>(allExpandScreenIds.size()));
        return DMError::DM_OK;
    }

    DMError ret = abstractScreenController_->StopScreens(allExpandScreenIds, ScreenCombination::SCREEN_EXPAND);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "stop expand failed.");
        return ret;
    }

    return DMError::DM_OK;
}

DMError DisplayManagerService::SetScreenActiveMode(ScreenId screenId, uint32_t modeId)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "set screen active permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:SetScreenActiveMode(%" PRIu64 ", %u)", screenId, modeId);
    return abstractScreenController_->SetScreenActiveMode(screenId, modeId);
}

DMError DisplayManagerService::SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "set virtual pixel permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:SetVirtualPixelRatio(%" PRIu64 ", %f)", screenId,
        virtualPixelRatio);
    return abstractScreenController_->SetVirtualPixelRatio(screenId, virtualPixelRatio);
}

DMError DisplayManagerService::IsScreenRotationLocked(bool& isLocked)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "is screen rotation locked permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    isLocked = ScreenRotationController::IsScreenRotationLocked();
    return DMError::DM_OK;
}

DMError DisplayManagerService::SetScreenRotationLocked(bool isLocked)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "set screen rotation locked permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    return ScreenRotationController::SetScreenRotationLocked(isLocked);
}

DMError DisplayManagerService::SetScreenRotationLockedFromJs(bool isLocked)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "set screen rotation locked from js permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    return ScreenRotationController::SetScreenRotationLocked(isLocked);
}

void DisplayManagerService::SetGravitySensorSubscriptionEnabled()
{
    if (!isAutoRotationOpen_) {
        TLOGE(WmsLogTag::DMS, "autoRotation is not open");
        ScreenRotationController::Init();
        return;
    }
    SensorConnector::SubscribeRotationSensor();
}

sptr<CutoutInfo> DisplayManagerService::GetCutoutInfo(DisplayId displayId)
{
    return displayCutoutController_->GetCutoutInfo(displayId);
}

void DisplayManagerService::NotifyPrivateWindowStateChanged(bool hasPrivate)
{
    DisplayManagerAgentController::GetInstance().NotifyPrivateWindowStateChanged(hasPrivate);
}

std::vector<DisplayPhysicalResolution> DisplayManagerService::GetAllDisplayPhysicalResolution()
{
    if (allDisplayPhysicalResolution_.empty()) {
        sptr<DisplayInfo> displayInfo = DisplayManagerService::GetDefaultDisplayInfo();
        if (displayInfo == nullptr) {
            TLOGE(WmsLogTag::DMS, "default display null");
            return allDisplayPhysicalResolution_;
        }
        DisplayPhysicalResolution defaultResolution;
        defaultResolution.foldDisplayMode_ = FoldDisplayMode::UNKNOWN;
        defaultResolution.physicalWidth_ = static_cast<uint32_t>(displayInfo->GetWidth());
        defaultResolution.physicalHeight_ = static_cast<uint32_t>(displayInfo->GetHeight());
        allDisplayPhysicalResolution_.emplace_back(defaultResolution);
    }
    return allDisplayPhysicalResolution_;
}

bool DisplayManagerService::SetVirtualScreenAsDefault(ScreenId screenId)
{
    TLOGI(WmsLogTag::DMS, "Enter, screenId: %{public}" PRIu64, screenId);
    return abstractScreenController_->SetVirtualScreenAsDefault(screenId);
}
} // namespace OHOS::Rosen
