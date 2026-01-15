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

#include "screen_session_manager_client.h"

#include <hitrace_meter.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>
#include <transaction/rs_transaction.h>
#include <transaction/rs_interfaces.h>
#include "dm_common.h"
#include "pipeline/rs_node_map.h"
#include "window_manager_hilog.h"
#include "fold_screen_controller/super_fold_state_manager.h"
#include "fold_screen_state_internel.h"
#include "rs_adapter.h"

namespace OHOS::Rosen {
namespace {
constexpr int LINE_WIDTH = 30;
} // namespace

WM_IMPLEMENT_SINGLE_INSTANCE(ScreenSessionManagerClient)

void ScreenSessionManagerClient::ConnectToServer()
{
    if (screenSessionManager_) {
        TLOGI(WmsLogTag::DMS, "Success to get screen session manager proxy");
        return;
    }
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityMgr) {
        TLOGE(WmsLogTag::DMS, "Failed to get system ability mgr");
        return;
    }

    auto remoteObject = systemAbilityMgr->GetSystemAbility(DISPLAY_MANAGER_SERVICE_SA_ID);
    if (!remoteObject) {
        TLOGE(WmsLogTag::DMS, "Failed to get display manager service");
        return;
    }

    screenSessionManager_ = iface_cast<IScreenSessionManager>(remoteObject);
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "Failed to get screen session manager proxy");
        return;
    }
    screenSessionManager_->SetClient(this);
}

void ScreenSessionManagerClient::RegisterScreenConnectionListener(IScreenConnectionListener* listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to register screen connection listener, listener is null");
        return;
    }

    screenConnectionListener_ = listener;
    ConnectToServer();
    TLOGI(WmsLogTag::DMS, "Success to register screen connection listener");
}

void ScreenSessionManagerClient::RegisterScreenConnectionChangeListener(
    const sptr<IScreenConnectionChangeListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed: listener is null");
        return;
    }
    screenConnectionChangeListener_ = listener;
    TLOGI(WmsLogTag::DMS, "Success");
}

void ScreenSessionManagerClient::NotifyScreenConnect(const sptr<ScreenSession>& screenSession)
{
    if (screenConnectionListener_) {
        screenConnectionListener_->OnScreenConnected(screenSession);
    }
    if (screenConnectionChangeListener_) {
        screenConnectionChangeListener_->OnScreenConnected(screenSession);
    }
}

void ScreenSessionManagerClient::NotifyScreenDisconnect(const sptr<ScreenSession>& screenSession)
{
    if (screenConnectionListener_) {
        screenConnectionListener_->OnScreenDisconnected(screenSession);
    }
    if (screenConnectionChangeListener_) {
        screenConnectionChangeListener_->OnScreenDisconnected(screenSession);
    }
}

bool ScreenSessionManagerClient::CheckIfNeedConnectScreen(SessionOption option)
{
    if (option.rsId_ == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "rsId is invalid");
        return false;
    }
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is nullptr");
        return false;
    }
    if (screenSessionManager_->GetScreenProperty(option.screenId_).GetScreenType() == ScreenType::VIRTUAL) {
        if (option.name_ == "HiCar" || option.name_ == "SuperLauncher" || option.name_ == "CastEngine" ||
            option.name_ == "DevEcoViewer" || option.innerName_ == "CustomScbScreen" || option.name_ == "CeliaView" ||
            option.name_ == "PadWithCar" || option.name_ == "CooperationExtend") {
            TLOGI(WmsLogTag::DMS, "HiCar or SuperLauncher or CastEngine or DevEcoViewer or CeliaView, "
                "need to connect the screen");
            return true;
        } else {
            TLOGE(WmsLogTag::DMS, "ScreenType is virtual, no need to connect the screen");
            return false;
        }
    }
    return true;
}

void ScreenSessionManagerClient::OnScreenConnectionChanged(SessionOption option, ScreenEvent screenEvent)
{
    TLOGI(WmsLogTag::DMS,
        "sId: %{public}" PRIu64 " sEvent: %{public}d rsId: %{public}" PRIu64 " name: %{public}s iName: %{public}s",
        option.screenId_, static_cast<int>(screenEvent), option.rsId_, option.name_.c_str(), option.innerName_.c_str());
    std::unique_lock<std::mutex> lock(screenEventMutex_);
    auto iter = connectedScreenSet_.find(option.screenId_);
    if (iter == connectedScreenSet_.end()) {
        if (screenEvent == ScreenEvent::DISCONNECTED) {
            TLOGW(WmsLogTag::DMS,
                "discard disconnect task, sid:%{public}" PRIu64" sEvent:%{public}d rsId: %{public}" PRIu64,
                option.screenId_, static_cast<int>(screenEvent), option.rsId_);
            return;
        }
    } else {
        if (screenEvent == ScreenEvent::CONNECTED) {
            TLOGE(WmsLogTag::DMS,
                "discard connect task, sid:%{public}" PRIu64" sEvent:%{public}d rsId: %{public}" PRIu64,
                option.screenId_, static_cast<int>(screenEvent), option.rsId_);
            return;
        }
    }
    if (screenEvent == ScreenEvent::CONNECTED) {
        TLOGD(WmsLogTag::DMS, "ScreenSessionManagerClient processing callback, isRotationLocked: %{public}d"
              "rotation: %{public}d, rotationOrientationMap: %{public}s",
              option.isRotationLocked_, option.rotation_, MapToString(option.rotationOrientationMap_).c_str());
        if (HandleScreenConnection(option)) {
            connectedScreenSet_.insert(option.screenId_);
            TLOGI(WmsLogTag::DMS,
                "screen event wait connecting, sId: %{public}" PRIu64 " sEvent: %{public}d rsId: %{public}" PRIu64,
                option.screenId_, static_cast<int>(screenEvent), option.rsId_);
        } else {
            TLOGI(WmsLogTag::DMS,
                "screen event not conent, sId: %{public}" PRIu64 " sEvent: %{public}d rsId: %{public}" PRIu64,
                option.screenId_, static_cast<int>(screenEvent), option.rsId_);
        }
    }
    if (screenEvent == ScreenEvent::DISCONNECTED) {
        HandleScreenDisconnectEvent(option, screenEvent);
    }
}

void ScreenSessionManagerClient::RegisterTentModeChangeListener(ITentModeListener* listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to register tent mode listener, listener is null");
        return;
    }

    tentModeListener_ = listener;
    ConnectToServer();

    OnTentModeChange(tentMode_);
    TLOGI(WmsLogTag::DMS, "Success to register tent mode connection listener");
}

void ScreenSessionManagerClient::OnTentModeChange(TentMode tentMode)
{
    TLOGD(WmsLogTag::DMS, "tentMode callback trigger");
    tentMode_ = tentMode;
    if (tentModeListener_) {
        tentModeListener_->OnTentModeChange(tentMode);
    }
}

void ScreenSessionManagerClient::HandleScreenDisconnectEvent(SessionOption option, ScreenEvent screenEvent)
{
    if (HandleScreenDisconnection(option)) {
        connectedScreenSet_.erase(option.screenId_);
        TLOGI(WmsLogTag::DMS,
            "screen event wait disconnecting, sId: %{public}" PRIu64 " sEvent: %{public}d rsId: %{public}" PRIu64,
            option.screenId_, static_cast<int>(screenEvent), option.rsId_);
    } else {
        TLOGI(WmsLogTag::DMS,
            "screen event not disconent, sId: %{public}" PRIu64 " sEvent: %{public}d rsId: %{public}" PRIu64,
            option.screenId_, static_cast<int>(screenEvent), option.rsId_);
    }
}

void ScreenSessionManagerClient::ExtraDestroyScreen(ScreenId screenId)
{
    sptr<ScreenSession> screenSession = nullptr;
    {
        std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
        for (const auto& iter : extraScreenSessionMap_) {
            sptr<ScreenSession> tempScreenSession = iter.second;
            if (tempScreenSession != nullptr) {
                if (tempScreenSession->GetScreenId() == screenId) {
                    screenSession = tempScreenSession;
                    break;
                }
            }
        }
    }
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "extra screenSession is null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "ScreenId:%{public}" PRIu64 ", rsId:%{public}" PRIu64,
        screenSession->GetScreenId(), screenSession->GetRSScreenId());
    {
        std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
        screenSession->DestroyScreenScene();
        extraScreenSessionMap_.erase(screenId);
    }
    TLOGI(WmsLogTag::DMS, "end");
}

void ScreenSessionManagerClient::OnScreenExtendChanged(ScreenId mainScreenId, ScreenId extendScreenId)
{
    auto screenSession = GetScreenSession(mainScreenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "mainScreenId=%{public}" PRIu64" extendScreenId=%{public}" PRIu64, mainScreenId,
        extendScreenId);
    screenSession->ScreenExtendChange(mainScreenId, extendScreenId);
}

sptr<ScreenSession> ScreenSessionManagerClient::GetScreenSession(ScreenId screenId) const
{
    std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
    auto iter = screenSessionMap_.find(screenId);
    if (iter == screenSessionMap_.end()) {
        TLOGE_LIMITN_MIN(WmsLogTag::DMS, THREE_TIMES, "Error found screen session with id: %{public}" PRIu64, screenId);
        return nullptr;
    }
    return iter->second;
}

sptr<ScreenSession> ScreenSessionManagerClient::GetScreenSessionExtra(ScreenId screenId) const
{
    std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
    auto iter = extraScreenSessionMap_.find(screenId);
    if (iter == extraScreenSessionMap_.end()) {
        TLOGE(WmsLogTag::DMS, "Error found extra screen session with id: %{public}" PRIu64, screenId);
        return nullptr;
    }
    return iter->second;
}

void ScreenSessionManagerClient::OnPropertyChanged(ScreenId screenId,
    const ScreenProperty& property, ScreenPropertyChangeReason reason)
{
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    screenSession->PropertyChange(property, reason);
}

void ScreenSessionManagerClient::OnScreenPropertyChanged(ScreenId screenId, float rotation, RRect bounds)
{
    TLOGI(WmsLogTag::DMS, "OnScreenPropertyChanged,screenId: %{public}" PRIu64
        ", rotation:%{public}f, width:%{public}f, height:%{public}f", screenId,
        rotation, bounds.rect_.GetWidth(), bounds.rect_.GetHeight());
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    screenSession->ModifyScreenPropertyWithLock(rotation, bounds);
    if (currentstate_ != SuperFoldStatus::KEYBOARD) {
        screenSession->SetValidHeight(bounds.rect_.GetHeight());
        screenSession->SetValidWidth(bounds.rect_.GetWidth());
    }
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->SyncScreenPropertyChangedToServer(screenId, screenSession->GetScreenProperty());
}

bool ScreenSessionManagerClient::OnFoldPropertyChange(ScreenId screenId, const ScreenProperty& property,
    ScreenPropertyChangeReason reason, FoldDisplayMode displayMode, ScreenProperty& screenProperty)
{
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return false;
    }
    screenProperty = screenSession->GetScreenProperty();
    screenProperty.SetDpiPhyBounds(property.GetPhyWidth(), property.GetPhyHeight());
    screenProperty.SetPhyBounds(property.GetPhyBounds());
    screenProperty.SetBounds(property.GetBounds());
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        DisplayOrientation deviceOrientation =
            screenSession->CalcDeviceOrientation(screenProperty.GetScreenRotation(), displayMode);
        screenProperty.SetDisplayOrientation(deviceOrientation);
        screenProperty.SetDeviceOrientation(deviceOrientation);
        screenProperty.SetScreenAreaOffsetY(property.GetScreenAreaOffsetY());
        screenProperty.SetScreenAreaHeight(property.GetScreenAreaHeight());
        screenProperty.SetScreenAreaWidth(property.GetScreenAreaWidth());
        screenProperty.SetInputOffset(property.GetInputOffsetX(), property.GetInputOffsetY());
    }
    screenProperty.SetPhysicalTouchBounds(screenSession->GetRotationCorrection(displayMode));
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice() || FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
        screenProperty.SetValidHeight(screenProperty.GetBounds().rect_.GetHeight());
        screenProperty.SetValidWidth(screenProperty.GetBounds().rect_.GetWidth());
    }

    screenSession->PropertyChange(screenProperty, reason);
    return true;
}

void ScreenSessionManagerClient::OnPowerStatusChanged(DisplayPowerEvent event, EventStatus status,
    PowerStateChangeReason reason)
{
    sptr<ScreenSession> screenSession;
    {
        std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
        if (screenSessionMap_.empty()) {
            TLOGE(WmsLogTag::DMS, "[UL_POWER]screenSessionMap_ is empty");
            return;
        }
        screenSession = screenSessionMap_.begin()->second;
    }
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]screenSession is null");
        return;
    }
    screenSession->PowerStatusChange(event, status, reason);
}

void ScreenSessionManagerClient::OnSensorRotationChanged(ScreenId screenId, float sensorRotation, bool isSwitchUser)
{
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    screenSession->SensorRotationChange(sensorRotation, isSwitchUser);
}

void ScreenSessionManagerClient::OnHoverStatusChanged(ScreenId screenId, int32_t hoverStatus, bool needRotate)
{
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    screenSession->HandleHoverStatusChange(hoverStatus, needRotate);
}

void ScreenSessionManagerClient::OnScreenOrientationChanged(ScreenId screenId, float screenOrientation)
{
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    screenSession->ScreenOrientationChange(screenOrientation);
}

void ScreenSessionManagerClient::OnScreenRotationLockedChanged(ScreenId screenId, bool isLocked)
{
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    screenSession->SetScreenRotationLocked(isLocked);
}

void ScreenSessionManagerClient::OnCameraBackSelfieChanged(ScreenId screenId, bool isCameraBackSelfie)
{
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    screenSession->HandleCameraBackSelfieChange(isCameraBackSelfie);
}

void ScreenSessionManagerClient::RegisterDisplayChangeListener(const sptr<IDisplayChangeListener>& listener)
{
    displayChangeListener_ = listener;
}

void ScreenSessionManagerClient::RegisterSwitchingToAnotherUserFunction(std::function<void()>&& func)
{
    switchingToAnotherUserFunc_ = func;
}

void ScreenSessionManagerClient::OnDisplayStateChanged(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type)
{
    if (displayChangeListener_) {
        displayChangeListener_->OnDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    }
}

void ScreenSessionManagerClient::OnUpdateFoldDisplayMode(FoldDisplayMode displayMode)
{
    displayMode_ = displayMode;
}

void ScreenSessionManagerClient::OnGetSurfaceNodeIdsFromMissionIdsChanged(std::vector<uint64_t>& missionIds,
    std::vector<uint64_t>& surfaceNodeIds, const std::vector<uint32_t>& needWindowTypeList, bool isNeedForceCheck)
{
    if (displayChangeListener_) {
        displayChangeListener_->OnGetSurfaceNodeIdsFromMissionIds(missionIds, surfaceNodeIds,
            needWindowTypeList, isNeedForceCheck);
    }
}

void ScreenSessionManagerClient::OnSetSurfaceNodeIdsChanged(DisplayId displayId,
    const std::vector<uint64_t>& surfaceNodeIds)
{
    if (displayChangeListener_) {
        displayChangeListener_->OnSetSurfaceNodeIds(displayId, surfaceNodeIds);
    }
}

void ScreenSessionManagerClient::OnVirtualScreenDisconnected(DisplayId displayId)
{
    if (displayChangeListener_) {
        displayChangeListener_->OnVirtualScreenDisconnected(displayId);
    }
}

void ScreenSessionManagerClient::OnScreenshot(DisplayId displayId)
{
    if (displayChangeListener_) {
        displayChangeListener_->OnScreenshot(displayId);
    }
}

void ScreenSessionManagerClient::OnImmersiveStateChanged(ScreenId screenId, bool& immersive)
{
    if (displayChangeListener_ != nullptr) {
        displayChangeListener_->OnImmersiveStateChange(screenId, immersive);
    }
}

std::map<ScreenId, ScreenProperty> ScreenSessionManagerClient::GetAllScreensProperties() const
{
    std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
    std::map<ScreenId, ScreenProperty> screensProperties;
    for (const auto& iter: screenSessionMap_) {
        if (iter.second == nullptr) {
            continue;
        }
        screensProperties[iter.first] = iter.second->GetScreenProperty();
    }
    return screensProperties;
}

FoldDisplayMode ScreenSessionManagerClient::GetFoldDisplayMode() const
{
    return displayMode_;
}

void ScreenSessionManagerClient::UpdateScreenRotationProperty(ScreenId screenId, const RRect& bounds,
    ScreenDirectionInfo directionInfo, ScreenPropertyChangeType screenPropertyChangeType)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->UpdateScreenDirectionInfo(screenId, directionInfo, screenPropertyChangeType, bounds);
    screenSessionManager_->UpdateScreenRotationProperty(screenId, bounds, directionInfo.notifyRotation_,
        screenPropertyChangeType);

    // not need update property to input manager
    if (screenPropertyChangeType == ScreenPropertyChangeType::ROTATION_END ||
        screenPropertyChangeType == ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY ||
        screenPropertyChangeType == ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY_NOT_NOTIFY) {
        return;
    }
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    auto foldDisplayMode = screenSessionManager_->GetFoldDisplayMode();
    screenSession->SetPhysicalRotation(directionInfo.phyRotation_);
    screenSession->SetScreenComponentRotation(directionInfo.screenRotation_);
    screenSession->UpdateToInputManager(bounds, directionInfo.notifyRotation_, directionInfo.rotation_,
        foldDisplayMode);
    screenSession->UpdateTouchBoundsAndOffset(foldDisplayMode);
    TLOGW(WmsLogTag::DMS, "superFoldStatus:%{public}d", currentstate_);
    if (currentstate_ != SuperFoldStatus::KEYBOARD) {
        screenSession->SetValidHeight(bounds.rect_.GetHeight());
        screenSession->SetValidWidth(bounds.rect_.GetWidth());
    }
}

void ScreenSessionManagerClient::SetDisplayNodeScreenId(ScreenId screenId, ScreenId displayNodeScreenId)
{
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    screenSession->SetDisplayNodeScreenId(displayNodeScreenId);
}

uint32_t ScreenSessionManagerClient::GetCurvedCompressionArea()
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return 0;
    }
    return screenSessionManager_->GetCurvedCompressionArea();
}

ScreenProperty ScreenSessionManagerClient::GetPhyScreenProperty(ScreenId screenId)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return {};
    }
    return screenSessionManager_->GetPhyScreenProperty(screenId);
}

__attribute__((no_sanitize("cfi"))) void ScreenSessionManagerClient::NotifyDisplayChangeInfoChanged(
    const sptr<DisplayChangeInfo>& info)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->NotifyDisplayChangeInfoChanged(info);
}

void ScreenSessionManagerClient::SetScreenPrivacyState(bool hasPrivate)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    TLOGD(WmsLogTag::DMS, "Begin calling the SetScreenPrivacyState() of screenSessionManager_, hasPrivate: %{public}d",
        hasPrivate);
    screenSessionManager_->SetScreenPrivacyState(hasPrivate);
    TLOGD(WmsLogTag::DMS, "End calling the SetScreenPrivacyState() of screenSessionManager_");
}

void ScreenSessionManagerClient::SetPrivacyStateByDisplayId(std::unordered_map<DisplayId, bool>& privacyBundleDisplayId)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    for (const auto& iter : privacyBundleDisplayId) {
        TLOGD(WmsLogTag::DMS, "displayId: %{public}" PRIu64" hasPrivate: %{public}d", iter.first, iter.second);
    }
    screenSessionManager_->SetPrivacyStateByDisplayId(privacyBundleDisplayId);
    TLOGD(WmsLogTag::DMS, "End calling the SetPrivacyStateByDisplayId");
}

void ScreenSessionManagerClient::SetScreenPrivacyWindowList(
    std::unordered_map<DisplayId, std::vector<std::string>>& privacyBundleList)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    for (const auto& iter : privacyBundleList) {
        TLOGD(WmsLogTag::DMS, "Begin calling the SetScreenPrivacyWindowList(), id: %{public}" PRIu64, iter.first);
        screenSessionManager_->SetScreenPrivacyWindowList(iter.first, iter.second);
    }
    TLOGD(WmsLogTag::DMS, "End calling the SetScreenPrivacyWindowList()");
}

void ScreenSessionManagerClient::UpdateAvailableArea(ScreenId screenId, DMRect area)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->UpdateAvailableArea(screenId, area);
}

void ScreenSessionManagerClient::UpdateSuperFoldAvailableArea(ScreenId screenId, DMRect bArea, DMRect cArea)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->UpdateSuperFoldAvailableArea(screenId, bArea, cArea);
}

void ScreenSessionManagerClient::UpdateSuperFoldExpandAvailableArea(ScreenId screenId, DMRect area)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->UpdateSuperFoldExpandAvailableArea(screenId, area);
}

int32_t ScreenSessionManagerClient::SetScreenOffDelayTime(int32_t delay)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return 0;
    }
    return screenSessionManager_->SetScreenOffDelayTime(delay);
}

int32_t ScreenSessionManagerClient::SetScreenOnDelayTime(int32_t delay)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return 0;
    }
    return screenSessionManager_->SetScreenOnDelayTime(delay);
}

void ScreenSessionManagerClient::SetCameraStatus(int32_t cameraStatus, int32_t cameraPosition)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    return screenSessionManager_->SetCameraStatus(cameraStatus, cameraPosition);
}

void ScreenSessionManagerClient::NotifyFoldToExpandCompletion(bool foldToExpand)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->NotifyFoldToExpandCompletion(foldToExpand);
}

void ScreenSessionManagerClient::NotifyScreenConnectCompletion(ScreenId screenId)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager is null");
        return;
    }
    screenSessionManager_->NotifyScreenConnectCompletion(screenId);
}

void ScreenSessionManagerClient::NotifyAodOpCompletion(AodOP operation, int32_t result)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager is null");
        return;
    }
    screenSessionManager_->NotifyAodOpCompletion(operation, result);
}

void ScreenSessionManagerClient::RecordEventFromScb(std::string description, bool needRecordEvent)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->RecordEventFromScb(description, needRecordEvent);
}

void ScreenSessionManagerClient::SwitchUserCallback(std::vector<int32_t> oldScbPids, int32_t currentScbPid)
{
    if (screenSessionManager_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    if (oldScbPids.size() == 0) {
        TLOGE(WmsLogTag::DMS, "oldScbPids size 0");
        return;
    }
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMapCopy;
    {
        std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
        screenSessionMapCopy = screenSessionMap_;
    }
    for (const auto& iter : screenSessionMapCopy) {
        ScreenId screenId = iter.first;
        sptr<ScreenSession> screenSession = iter.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is null");
            continue;
        }
        ScreenProperty screenProperty = screenSession->GetScreenProperty();
        RRect bounds = screenProperty.GetBounds();
        float rotation = screenSession->ConvertRotationToFloat(screenSession->GetRotation());
        if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
            UpdatePropertyWhenSwitchUser(screenSession, rotation, bounds, screenId);
        } else {
            screenSessionManager_->UpdateScreenRotationProperty(screenId, bounds, rotation,
                ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY, true);
        }
    }
    TLOGI(WmsLogTag::DMS, "switch user callback end");
}

void ScreenSessionManagerClient::SwitchingCurrentUser()
{
    if (screenSessionManager_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->SwitchUser();
    TLOGI(WmsLogTag::DMS, "switch to current user end");
}

void ScreenSessionManagerClient::DisconnectAllExternalScreen()
{
    ScreenId setScreenId = SCREEN_ID_INVALID;
    {
        std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
        for (auto sessionIt = screenSessionMap_.rbegin(); sessionIt != screenSessionMap_.rend(); ++sessionIt) {
            auto screenSession = sessionIt->second;
            if (screenSession == nullptr) {
                TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
                continue;
            }
            if (screenSession->GetScreenProperty().GetScreenType() ==
                ScreenType::REAL && screenSession->GetIsExtend()) {
                TLOGI(WmsLogTag::DMS, "disconnect extend screen, screenId = %{public}" PRIu64, sessionIt->first);
                screenSession->DestroyScreenScene();
                NotifyScreenDisconnect(screenSession);
                ScreenId screenId = sessionIt->first;
                screenSessionMap_.erase(screenId);
                setScreenId = screenId;
                screenSession->Disconnect();
                break;
            }
        }
    }
    if (setScreenId != SCREEN_ID_INVALID) {
        std::unique_lock<std::mutex> lock(screenEventMutex_);
        connectedScreenSet_.erase(setScreenId);
    }
}

FoldStatus ScreenSessionManagerClient::GetFoldStatus()
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return FoldStatus::UNKNOWN;
    }
    return screenSessionManager_->GetFoldStatus();
}

SuperFoldStatus ScreenSessionManagerClient::GetSuperFoldStatus()
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return SuperFoldStatus::UNKNOWN;
    }
    return screenSessionManager_->GetSuperFoldStatus();
}

float ScreenSessionManagerClient::GetSuperRotation()
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return -1.f;
    }
    return screenSessionManager_->GetSuperRotation();
}

void ScreenSessionManagerClient::SetLandscapeLockStatus(bool isLocked)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    return screenSessionManager_->SetLandscapeLockStatus(isLocked);
}

void ScreenSessionManagerClient::SetForceCloseHdr(ScreenId screenId, bool isForceCloseHdr)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    return screenSessionManager_->SetForceCloseHdr(screenId, isForceCloseHdr);
}

ExtendScreenConnectStatus ScreenSessionManagerClient::GetExtendScreenConnectStatus()
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return ExtendScreenConnectStatus::UNKNOWN;
    }
    return screenSessionManager_->GetExtendScreenConnectStatus();
}

std::shared_ptr<Media::PixelMap> ScreenSessionManagerClient::GetScreenSnapshot(ScreenId screenId,
    float scaleX, float scaleY)
{
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "get screen session is null");
        return nullptr;
    }
    return screenSession->GetScreenSnapshot(scaleX, scaleY);
}

DeviceScreenConfig ScreenSessionManagerClient::GetDeviceScreenConfig()
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return {};
    }
    return screenSessionManager_->GetDeviceScreenConfig();
}

sptr<ScreenSession> ScreenSessionManagerClient::GetScreenSessionById(const ScreenId id)
{
    std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
    auto iter = screenSessionMap_.find(id);
    if (iter == screenSessionMap_.end()) {
        return nullptr;
    }
    return iter->second;
}

std::shared_ptr<RSUIDirector> ScreenSessionManagerClient::GetRSUIDirector(ScreenId screenId)
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED(nullptr);
    if (screenId == SCREEN_ID_INVALID) {
        screenId = GetDefaultScreenId();
        TLOGW(WmsLogTag::WMS_SCB,
              "screenId is invalid, use default screenId: %{public}" PRIu64, screenId);
    }
    if (screenId == SCREEN_ID_INVALID) {
        TLOGW(WmsLogTag::WMS_SCB, "Default screenId is also invalid");
        return nullptr;
    }
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::WMS_SCB,
              "Screen session is null, screenId: %{public}" PRIu64, screenId);
        return nullptr;
    }
    auto rsUIDirector = screenSession->GetRSUIDirector();
    TLOGD(WmsLogTag::WMS_SCB,
          "%{public}s, screenId: %{public}" PRIu64,
          RSAdapterUtil::RSUIDirectorToStr(rsUIDirector).c_str(), screenId);
    return rsUIDirector;
}

std::shared_ptr<RSUIContext> ScreenSessionManagerClient::GetRSUIContext(ScreenId screenId)
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED(nullptr);
    auto rsUIDirector = GetRSUIDirector(screenId);
    auto rsUIContext = rsUIDirector ? rsUIDirector->GetRSUIContext() : nullptr;
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s, screenId: %{public}" PRIu64,
          RSAdapterUtil::RSUIContextToStr(rsUIContext).c_str(), screenId);
    return rsUIContext;
}

ScreenId ScreenSessionManagerClient::GetDefaultScreenId()
{
    std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
    auto iter = screenSessionMap_.begin();
    if (iter != screenSessionMap_.end()) {
        return iter->first;
    }
    return SCREEN_ID_INVALID;
}

bool ScreenSessionManagerClient::IsFoldable()
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return false;
    }
    return screenSessionManager_->IsFoldable();
}

void ScreenSessionManagerClient::SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio)
{
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screen session is null");
        return;
    }
    if (screenSession->isScreenGroup_) {
        TLOGE(WmsLogTag::DMS, "cannot set virtual pixel ratio to the combination. screen: %{public}" PRIu64, screenId);
        return;
    }
    screenSession->SetScreenSceneDpi(virtualPixelRatio);
}

void ScreenSessionManagerClient::UpdateDisplayHookInfo(int32_t uid, bool enable, const DMHookInfo& hookInfo)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->UpdateDisplayHookInfo(uid, enable, hookInfo);
}

void ScreenSessionManagerClient::GetDisplayHookInfo(int32_t uid, DMHookInfo& hookInfo) const
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->GetDisplayHookInfo(uid, hookInfo);
}

void ScreenSessionManagerClient::NotifyIsFullScreenInForceSplitMode(int32_t uid, bool isFullScreen)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->NotifyIsFullScreenInForceSplitMode(uid, isFullScreen);
}

void ScreenSessionManagerClient::OnFoldStatusChangedReportUE(const std::vector<std::string>& screenFoldInfo)
{
    if (displayChangeListener_) {
        displayChangeListener_->OnScreenFoldStatusChanged(screenFoldInfo);
    }
}

void ScreenSessionManagerClient::UpdateDisplayScale(ScreenId id, float scaleX, float scaleY, float pivotX, float pivotY,
                                                    float translateX, float translateY)
{
    auto session = GetScreenSession(id);
    if (session == nullptr) {
        TLOGE(WmsLogTag::DMS, "session is null");
        return;
    }
    auto displayNode = session->GetDisplayNode();
    if (displayNode == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayNode is null");
        return;
    }
    TLOGW(WmsLogTag::DMS, "scale [%{public}f, %{public}f] translate [%{public}f, %{public}f]", scaleX, scaleY,
          translateX, translateY);
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
                      "ssmc:UpdateDisplayScale(ScreenId = %" PRIu64
                      " scaleX=%f, scaleY=%f, pivotX=%f, pivotY=%f, translateX=%f, translateY=%f",
                      id, scaleX, scaleY, pivotX, pivotY, translateX, translateY);
    session->SetScreenScale(scaleX, scaleY, pivotX, pivotY, translateX, translateY);
    session->PropertyChange(session->GetScreenProperty(), ScreenPropertyChangeReason::ACCESS_INFO_CHANGE);
}

void ScreenSessionManagerClient::ScreenCaptureNotify(ScreenId mainScreenId, int32_t uid, const std::string& clientName)
{
    sptr<ScreenSession> screenSession = GetScreenSession(mainScreenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screen session is null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "capture screenId: %{public}" PRIu64", uid=%{public}d", mainScreenId, uid);
    screenSession->ScreenCaptureNotify(mainScreenId, uid, clientName);
}

void ScreenSessionManagerClient::OnSuperFoldStatusChanged(ScreenId screenId, SuperFoldStatus superFoldStatus)
{
    currentstate_ = superFoldStatus;
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "screenId=%{public}" PRIu64 " superFoldStatus=%{public}d", screenId,
        static_cast<uint32_t>(superFoldStatus));
    screenSession->SuperFoldStatusChange(screenId, superFoldStatus);
}

void ScreenSessionManagerClient::OnSecondaryReflexionChanged(ScreenId screenId, bool isSecondaryReflexion)
{
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "screenId=%{public}" PRIu64 " isSecondaryReflexion=%{public}d", screenId,
        isSecondaryReflexion);
    screenSession->SecondaryReflexionChange(screenId, isSecondaryReflexion);
}

void ScreenSessionManagerClient::OnExtendScreenConnectStatusChanged(ScreenId screenId,
    ExtendScreenConnectStatus extendScreenConnectStatus)
{
    auto screenSession = GetScreenSession(GetDefaultScreenId());
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "screenId=%{public}" PRIu64 " extendScreenConnectStatus=%{public}d", screenId,
        static_cast<uint32_t>(extendScreenConnectStatus));
    screenSession->ExtendScreenConnectStatusChange(screenId, extendScreenConnectStatus);
}

void ScreenSessionManagerClient::UpdatePropertyWhenSwitchUser(const sptr <ScreenSession>& screenSession,
    float rotation, RRect bounds, ScreenId screenId)
{
    currentstate_ = GetSuperFoldStatus();
    screenSession->SetPointerActiveWidth(0);
    screenSession->SetPointerActiveHeight(0);
    screenSession->UpdateToInputManager(bounds, static_cast<int>(rotation), static_cast<int>(rotation),
        FoldDisplayMode::UNKNOWN);
    screenSession->SetPhysicalRotation(rotation);
    screenSession->SetScreenComponentRotation(rotation);
    ScreenDirectionInfo directionInfo;
    directionInfo.screenRotation_ = static_cast<int32_t>(rotation);
    directionInfo.rotation_ = static_cast<int32_t>(rotation);
    directionInfo.phyRotation_ = static_cast<int32_t>(rotation);
    screenSessionManager_->UpdateScreenDirectionInfo(screenId, directionInfo,
        ScreenPropertyChangeType::UNSPECIFIED, bounds);
    screenSessionManager_->UpdateScreenRotationProperty(screenId, bounds, rotation,
        ScreenPropertyChangeType::UNSPECIFIED, true);
    UpdateWidthAndHeight(screenSession, &bounds, screenId);
    TLOGI(WmsLogTag::DMS, "get property by screenId=%{public}" PRIu64 ", "
        "bounds width=%{public}f, bounds height=%{public}f, "
        "pointerActiveWidth=%{public}u, pointerActiveHeight=%{public}u, "
        "validWidth=%{public}d, validHeight=%{public}d, "
        "screenAreaWidth=%{public}d, screenAreaHeight=%{public}d",
        screenId, bounds.rect_.GetWidth(), bounds.rect_.GetHeight(),
        screenSession->GetPointerActiveWidth(), screenSession->GetPointerActiveHeight(),
        screenSession->GetValidWidth(), screenSession->GetValidHeight(),
        screenSession->GetScreenAreaWidth(), screenSession->GetScreenAreaHeight());
}

void ScreenSessionManagerClient::UpdateWidthAndHeight(const sptr<ScreenSession>& screenSession,
    const RRect* bounds, ScreenId screenId)
{
    if (screenSession == nullptr || bounds == nullptr) {
        TLOGE(WmsLogTag::DMS, "Invalid parameters: screenSession or bounds is null");
        return;
    }
    ScreenProperty property = screenSessionManager_->GetScreenProperty(screenId);
    if (property.GetValidHeight() == UINT32_MAX || property.GetValidHeight() == 0) {
        TLOGW(WmsLogTag::DMS, "invalid property, validheight is bounds");
        screenSession->SetValidHeight(bounds->rect_.GetHeight());
    } else {
        screenSession->SetValidHeight(property.GetValidHeight());
    }
    if (property.GetValidWidth() == UINT32_MAX || property.GetValidWidth() == 0) {
        TLOGW(WmsLogTag::DMS, "invalid property, validwidth is bounds");
        screenSession->SetValidWidth(bounds->rect_.GetWidth());
    } else {
        screenSession->SetValidWidth(property.GetValidWidth());
    }
    if (property.GetScreenAreaWidth() == 0) {
        TLOGW(WmsLogTag::DMS, "invalid property, screenAreaWidth is bounds");
        screenSession->SetScreenAreaWidth(bounds->rect_.GetWidth());
    } else {
        screenSession->SetScreenAreaWidth(property.GetScreenAreaWidth());
    }
    if (property.GetScreenAreaHeight() == 0) {
        TLOGW(WmsLogTag::DMS, "invalid property, screenAreaHeight is bounds");
        screenSession->SetScreenAreaHeight(bounds->rect_.GetHeight());
    } else {
        screenSession->SetScreenAreaHeight(property.GetScreenAreaHeight());
    }
}

void ScreenSessionManagerClient::NotifyClientScreenConnect(sptr<ScreenSession>& screenSession)
{
    NotifyScreenConnect(screenSession);
    if (screenConnectionListener_) {
        TLOGW(WmsLogTag::DMS,  "screenId: %{public}" PRIu64 " density: %{public}f ",
            screenSession->GetScreenId(), screenSession->GetScreenProperty().GetDensity());
        screenSession->SetScreenSceneDpi(screenSession->GetScreenProperty().GetDensity());
    }
    screenSession->Connect();
}

bool ScreenSessionManagerClient::HandleScreenConnection(SessionOption option)
{
    if (!CheckIfNeedConnectScreen(option)) {
        TLOGW(WmsLogTag::DMS, "no need to connect the screen");
        return false;
    }
    sptr<ScreenSession> screenSession = nullptr;
    auto iter = screenSessionMap_.find(option.screenId_);
    if (iter != screenSessionMap_.end() && iter->second != nullptr) {
        TLOGW(WmsLogTag::DMS, "screen session has exist.");
        screenSession = iter->second;
    }
    ScreenSessionConfig config = {
        .screenId = option.screenId_,
        .rsId = option.rsId_,
        .name = option.name_,
        .innerName = option.innerName_,
    };
    config.property = screenSessionManager_->GetScreenProperty(option.screenId_);
    TLOGW(WmsLogTag::DMS, "width:%{public}f, height=%{public}f",
        config.property.GetBounds().rect_.GetWidth(), config.property.GetBounds().rect_.GetHeight());
    config.displayNode = screenSessionManager_->GetDisplayNode(option.screenId_);
    if (screenSession == nullptr) {
        screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    } else {
        screenSession->SetScreenProperty(config.property);
        screenSession->SetDisplayNode(config.displayNode);
    }
    screenSession->SetScreenCombination(screenSessionManager_->GetScreenCombination(option.screenId_));
    screenSession->SetIsExtend(option.isExtend_);
    screenSession->SetIsRealScreen(screenSessionManager_->GetIsRealScreen(option.screenId_));
    {
        std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
        screenSessionMap_[option.screenId_] = screenSession;
        extraScreenSessionMap_[option.screenId_] = screenSession;
    }
    screenSession->SetRotationCorrectionMap(option.rotationCorrectionMap_);
    screenSession->SetSupportsFocus(option.supportsFocus_);
    screenSession->SetUniqueRotationLock(option.isRotationLocked_);
    screenSession->SetUniqueRotation(option.rotation_);
    if (screenSession->GetUniqueRotationOrientationMap().size() != ROTATION_NUM) {
        screenSession->SetUniqueRotationOrientationMap(option.rotationOrientationMap_);
    }
    TLOGD(WmsLogTag::DMS, "Set unique screen rotation property in screenSession,"
          "isUniqueRotationLocked: %{public}d, uniqueRotation: %{public}d"
          "uniqueRotationOrientationMap: %{public}s",
          screenSession->GetUniqueRotationLock(), screenSession->GetUniqueRotation(),
          MapToString(screenSession->GetUniqueRotationOrientationMap()).c_str());

    NotifyClientScreenConnect(screenSession);
    return true;
}

bool ScreenSessionManagerClient::HandleScreenDisconnection(SessionOption option)
{
    auto screenSession = GetScreenSession(option.screenId_);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return false;
    }
    screenSession->DestroyScreenScene();
    NotifyScreenDisconnect(screenSession);
    {
        std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
        screenSessionMap_.erase(option.screenId_);
    }
    TLOGW(WmsLogTag::DMS, "disconnect screenId=%{public}" PRIu64, screenSession->GetScreenId());
    screenSession->Disconnect();
    return true;
}

bool ScreenSessionManagerClient::OnCreateScreenSessionOnly(ScreenId screenId, ScreenId rsId,
    const std::string& name, bool isExtend)
{
    sptr<ScreenSession> screenSession = nullptr;
    ScreenSessionConfig config = {
        .screenId = screenId,
        .rsId = rsId,
        .name = name,
    };
    {
        std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
        auto iter = screenSessionMap_.find(screenId);
        TLOGW(WmsLogTag::DMS, "screenId=%{public}" PRIu64" screen session size:%{public}d",
            screenId, static_cast<int>(screenSessionMap_.size()));

        if (iter != screenSessionMap_.end() && iter->second != nullptr) {
            screenSession = iter->second;
            TLOGW(WmsLogTag::DMS, "screen session has exist.");
        }
    }
    if (screenSessionManager_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create screen session, screenSessionManager_ is null");
        return false;
    }
    config.displayNode = screenSessionManager_->GetDisplayNode(screenId);
    if (screenSession == nullptr) {
        config.property = screenSessionManager_->GetScreenProperty(screenId);
        screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    } else {
        screenSession->SetDisplayNode(config.displayNode);
    }
    screenSession->SetScreenCombination(screenSessionManager_->GetScreenCombination(screenId));
    screenSession->SetIsExtend(isExtend);
    screenSession->SetIsRealScreen(true);
    {
        std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
        screenSessionMap_[screenId] = screenSession;
        extraScreenSessionMap_[screenId] = screenSession;
    }
    return true;
}

/* main external */
bool ScreenSessionManagerClient::OnExtendDisplayNodeChange(ScreenId mainScreenId, ScreenId extendScreenId)
{
    auto innerScreen = GetScreenSession(mainScreenId);
    auto externalScreen = GetScreenSession(extendScreenId);
    TLOGW(WmsLogTag::DMS, "mainScreenId=%{public}" PRIu64"; extendScreenId=%{public}" PRIu64,
        mainScreenId, extendScreenId);
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "param is null");
        return false;
    }
    auto mainNode = innerScreen->GetDisplayNode();
    auto extendNode = externalScreen->GetDisplayNode();
    if (mainNode == nullptr || extendNode == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayNode is null");
        return false;
    }
    ScreenId innerRSId = innerScreen->GetRSScreenId();
    ScreenId externalRSId = externalScreen->GetRSScreenId();
    std::ostringstream oss;
    oss << "innerScreen before screenId: " << mainScreenId
        << ", rsId: " << innerRSId
        << ", nodeId: " << mainNode->GetId()
        << ", externalScreen screenId: " << extendScreenId
        << ", rsId: " << externalRSId
        << ", nodeId: " << extendNode->GetId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    /* change displayNode */
    mainNode->SetScreenId(externalRSId);
    extendNode->SetScreenId(innerRSId);

    /* change rsId */
    innerScreen->SetRSScreenId(externalRSId);
    externalScreen->SetRSScreenId(innerRSId);
    RSTransactionAdapter::FlushImplicitTransaction(
        {innerScreen->GetRSUIContext(), externalScreen->GetRSUIContext()});
    oss.str("");
    oss << "innerScreen after screenId: " << mainScreenId
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", nodeId: " << mainNode->GetId()
        << ", externalScreen screenId: " << extendScreenId
        << ", rsId: " << externalScreen->GetRSScreenId()
        << ", nodeId: " << extendNode->GetId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    return true;
}

sptr<ScreenSession> ScreenSessionManagerClient::CreateTempScreenSession(
    ScreenId screenId, ScreenId rsId, const std::shared_ptr<RSDisplayNode>& displayNode)
{
    if (screenSessionManager_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create temp screen session, screenSessionManager_ is null");
        return nullptr;
    }
    ScreenSessionConfig config = {
        .screenId = screenId,
        .rsId = rsId,
        .displayNode = displayNode,
    };
    config.property = screenSessionManager_->GetScreenProperty(screenId);
    TLOGW(WmsLogTag::DMS, "CreateTempScreenSession width:%{public}f, height=%{public}f",
        config.property.GetBounds().rect_.GetWidth(), config.property.GetBounds().rect_.GetHeight());
    return sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
}

bool ScreenSessionManagerClient::OnMainDisplayNodeChange(ScreenId mainScreenId, ScreenId extendScreenId,
    ScreenId extendRSId)
{
    auto mainScreen = GetScreenSession(mainScreenId);
    if (mainScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "mainScreen is null");
        return false;
    }
    if (screenSessionManager_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return false;
    }
    std::ostringstream oss;
    oss << "mainScreen mainScreenId: " << mainScreenId
        << ", extendScreenId: " << extendScreenId
        << ", extendRSId: " << extendRSId;
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    auto mainNode = mainScreen->GetDisplayNode();
    auto extendNode = screenSessionManager_->GetDisplayNode(extendScreenId);
    if (mainNode == nullptr || extendNode == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayNode is null");
        return false;
    }
    /* change displayNode */
    ScreenId innerRSId = mainScreen->GetRSScreenId();
    oss.str("");
    oss << "mainScreen mainScreenId: " << mainScreen->GetScreenId()
        << ", mainRSId: " << innerRSId
        << ", mainNodeId: " << mainNode->GetId()
        << ", extendNodeId: " << extendNode->GetId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    auto tempScreenSession = CreateTempScreenSession(extendScreenId, innerRSId, extendNode);
    mainNode->SetScreenId(extendRSId);
    extendNode->SetScreenId(innerRSId);

    /* change rsId */
    mainScreen->SetRSScreenId(extendRSId);
    RSTransactionAdapter::FlushImplicitTransaction({mainNode, extendNode});
    return true;
}

void ScreenSessionManagerClient::SetScreenCombination(ScreenId mainScreenId, ScreenId extendScreenId,
    ScreenCombination extendCombination)
{
    sptr<ScreenSession> screenSession = GetScreenSession(mainScreenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screen session is null");
        return;
    }
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    screenSession = GetScreenSession(extendScreenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screen session is null");
        return;
    }
    screenSession->SetScreenCombination(extendCombination);
}
 
std::string ScreenSessionManagerClient::OnDumperClientScreenSessions()
{
    std::ostringstream oss;
    oss << "------------- Client Screen Infos -------------" << std::endl;
    {
        std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
        for (const auto& iter : screenSessionMap_) {
        if (iter.second == nullptr) {
            oss << std::left << std::setw(LINE_WIDTH) << "session: " << "nullptr" << std::endl;
            continue;
        }
        ScreenProperty screenProperty = iter.second->GetScreenProperty();
        oss << std::left << std::setw(LINE_WIDTH) << "Name: " << iter.second->GetName() << std::endl;
        oss << std::left << std::setw(LINE_WIDTH) << "ScreenId: " << iter.second->GetScreenId() << std::endl;
        oss << std::left << std::setw(LINE_WIDTH) << "RSScreenId: " << iter.second->GetRSScreenId() << std::endl;
        if (iter.second->GetDisplayNode() != nullptr) {
            oss << std::left << std::setw(LINE_WIDTH) << "DisplayNode: "
                << iter.second->GetDisplayNode()->GetId() << std::endl;
        } else {
            oss << std::left << std::setw(LINE_WIDTH) << "DisplayNode: " << "nullptr" << std::endl;
        }
        oss << std::left << std::setw(LINE_WIDTH) << "ScreenCombination: "
            << static_cast<int32_t>(iter.second->GetScreenCombination()) << std::endl;
        oss << std::left << std::setw(LINE_WIDTH) << "isExtend: "
            << (iter.second->GetIsExtend() ? "true" : "false") << std::endl;
        oss << std::left << std::setw(LINE_WIDTH) << "Orientation: "
            << static_cast<int32_t>(screenProperty.GetOrientation()) << std::endl;
        oss << std::left << std::setw(LINE_WIDTH) << "Rotation: "
            << static_cast<int32_t>(screenProperty.GetRotation()) << std::endl;
        oss << std::left << std::setw(LINE_WIDTH) << "Bounds<L,T,W,H>: "
            << screenProperty.GetBounds().rect_.GetLeft() << ", "
            << screenProperty.GetBounds().rect_.GetTop() << ", "
            << screenProperty.GetBounds().rect_.GetWidth() << ", "
            << screenProperty.GetBounds().rect_.GetHeight() << ", " << std::endl;
        oss << std::left << std::setw(LINE_WIDTH) << "PhyBounds<L,T,W,H>: "
            << screenProperty.GetPhyBounds().rect_.GetLeft() << ", "
            << screenProperty.GetPhyBounds().rect_.GetTop() << ", "
            << screenProperty.GetPhyBounds().rect_.GetWidth() << ", "
            << screenProperty.GetPhyBounds().rect_.GetHeight() << ", " << std::endl;
        oss << std::left << std::setw(LINE_WIDTH) << "AvailableArea<X,Y,W,H> "
            << screenProperty.GetAvailableArea().posX_ << ", "
            << screenProperty.GetAvailableArea().posY_ << ", "
            << screenProperty.GetAvailableArea().width_ << ", "
            << screenProperty.GetAvailableArea().height_ << ", " << std::endl;
    }
    }
    auto screenInfos = oss.str();
    TLOGW(WmsLogTag::DMS, "%{public}s", screenInfos.c_str());
    return screenInfos;
}

void ScreenSessionManagerClient::SetDefaultMultiScreenModeWhenSwitchUser()
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->SetDefaultMultiScreenModeWhenSwitchUser();
    return;
}

void ScreenSessionManagerClient::NotifyExtendScreenCreateFinish()
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    return screenSessionManager_->NotifyExtendScreenCreateFinish();
}

void ScreenSessionManagerClient::NotifyExtendScreenDestroyFinish()
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    return screenSessionManager_->NotifyExtendScreenDestroyFinish();
}

void ScreenSessionManagerClient::OnBeforeScreenPropertyChanged(FoldStatus foldStatus)
{
    sptr<ScreenSession> screenSession = nullptr;
    {
        std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
        if (screenSessionMap_.empty()) {
            TLOGE(WmsLogTag::DMS, "screenSessionMap_ is nullptr");
            return;
        }
        screenSession = screenSessionMap_.begin()->second;
    }
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "fold status %{public}d", foldStatus);
    screenSession->BeforeScreenPropertyChange(foldStatus);
}

void ScreenSessionManagerClient::OnScreenModeChanged(ScreenModeChangeEvent screenModeChangeEvent)
{
    auto screenSession = GetScreenSession(GetDefaultScreenId());
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "screenModeChangeEvent=%{public}d", static_cast<uint32_t>(screenModeChangeEvent));
    screenSession->ScreenModeChange(screenModeChangeEvent);
}

void ScreenSessionManagerClient::NotifyScreenMaskAppear()
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    return screenSessionManager_->NotifyScreenMaskAppear();
}

DMError ScreenSessionManagerClient::SetPrimaryDisplaySystemDpi(float dpi)
{
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    return screenSessionManager_->SetPrimaryDisplaySystemDpi(dpi);
}

void ScreenSessionManagerClient::FreezeScreen(ScreenId screenId, bool isFreeze)
{
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "get screen session is null, screenId is %{public}" PRIu64, screenId);
        return;
    }
    screenSession->FreezeScreen(isFreeze);
}

std::shared_ptr<Media::PixelMap> ScreenSessionManagerClient::GetScreenSnapshotWithAllWindows(ScreenId screenId,
    float scaleX, float scaleY, bool isNeedCheckDrmAndSurfaceLock)
{
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "get screen session is null, screenId is %{public}" PRIu64, screenId);
        return nullptr;
    }
    return screenSession->GetScreenSnapshotWithAllWindows(scaleX, scaleY, isNeedCheckDrmAndSurfaceLock);
}

void ScreenSessionManagerClient::NotifySwitchUserAnimationFinish(const std::string& description)
{
    TLOGI(WmsLogTag::DMS, "description: %{public}s", description.c_str());
    std::set<std::string> descriptionSetCopy;
    {
        std::shared_lock<std::shared_mutex> descriptionLock(animateFinishDescriptionSetMutex_);
        descriptionSetCopy = animateFinishDescriptionSet_;
    }
    {
        std::lock_guard<std::mutex> notificationLock(animateFinishNotificationSetMutex_);
        if (descriptionSetCopy.empty()) {
            return;
        }
        auto it = descriptionSetCopy.find(description);
        if (it == descriptionSetCopy.end()) {
            TLOGE(WmsLogTag::DMS, "not find description in map");
            return;
        }
        animateFinishNotificationSet_.insert(description);
        if (animateFinishNotificationSet_.size() != descriptionSetCopy.size()) {
            return;
        }
        // all description notified
        animateFinishNotificationSet_.clear();
    }
    TLOGI(WmsLogTag::DMS, "notify all animate finished");
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "screenSessionManager_ is null");
        return;
    }
    screenSessionManager_->NotifySwitchUserAnimationFinish();
}

void ScreenSessionManagerClient::RegisterSwitchUserAnimationNotification(const std::string& description)
{
    std::unique_lock<std::shared_mutex> lock(animateFinishDescriptionSetMutex_);
    auto it = animateFinishDescriptionSet_.find(description);
    if (it != animateFinishDescriptionSet_.end()) {
        TLOGE(WmsLogTag::DMS, "description: %{public}s already regist", description.c_str());
        return;
    }
    TLOGI(WmsLogTag::DMS, "description: %{public}s regist success", description.c_str());
    animateFinishDescriptionSet_.insert(description);
}

void ScreenSessionManagerClient::NotifySwitchUserAnimationFinishByWindow()
{
    {
        std::shared_lock<std::shared_mutex> descriptionLock(animateFinishDescriptionSetMutex_);
        if (!animateFinishDescriptionSet_.empty()) {
            TLOGI(WmsLogTag::DMS, "description set is not empty, will ignore notification by window");
            return;
        }
    }
    TLOGI(WmsLogTag::DMS, "notify animation finished by window");
    screenSessionManager_->NotifySwitchUserAnimationFinish();
}
void ScreenSessionManagerClient::OnAnimationFinish()
{
    std::lock_guard<std::mutex> lock(animateFinishNotificationSetMutex_);
    animateFinishNotificationSet_.clear();
}

void ScreenSessionManagerClient::SetInternalClipToBounds(ScreenId screenId, bool clipToBounds)
{
    sptr<ScreenSession> internalSession = nullptr;
    {
        std::lock_guard<std::mutex> lock(screenSessionMapMutex_);
        auto iter = screenSessionMap_.find(screenId);
        if (iter != screenSessionMap_.end()) {
            internalSession = iter->second;
        }
    }
    if (internalSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    auto displayNode = internalSession->GetDisplayNode();
    if (displayNode != nullptr) {
        TLOGI(WmsLogTag::DMS, "Screen %{public}" PRIu64" displayNode cliptobounds set to %{public}d",
            screenId, clipToBounds);
        displayNode->SetClipToBounds(clipToBounds);
        RSTransactionAdapter::FlushImplicitTransaction(displayNode);
    }
    TLOGI(WmsLogTag::DMS, "SetInternalClipToBounds end");
}

bool ScreenSessionManagerClient::GetSupportsFocus(DisplayId displayId)
{
    auto screenSession = GetScreenSession(displayId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "getSupportFocus screenSession is nullptr");
        return false;
    }
    bool supportsFocus = screenSession->GetSupportsFocus();
    TLOGD(WmsLogTag::DMS, "displayId:%{public}" PRIu64", supportsFocus:%{public}d", displayId, supportsFocus);
    return supportsFocus;
}
} // namespace OHOS::Rosen