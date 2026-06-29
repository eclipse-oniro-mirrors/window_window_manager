/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "picture_in_picture_manager.h"

#include <algorithm>
#include <functional>

#include "navigation_controller.h"
#include "parameters.h"
#include "picture_in_picture_controller.h"
#include "window_adapter.h"
#include "window_manager_hilog.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"

namespace OHOS {
namespace Rosen {
namespace {
const std::string ACTION_CLOSE = "close";
const std::string ACTION_REQUEST_CLOSE = "request_close";
const std::string ACTION_PANEL_CLOSE = "panel_close";
const std::string ACTION_DUMPSTER_CLOSE = "dumpster_close";
const std::string ACTION_PRE_RESTORE = "pre_restore";
const std::string ACTION_RESTORE = "restore";
const std::string ACTION_DESTROY = "destroy";
const std::string ACTION_PREPARE_SOURCE = "prepare_source";
const std::string ACTION_LOCATE_SOURCE = "locate_source";
const std::string ACTION_BACKGROUND_AUTO_START = "background_auto_start";

const std::map<std::string, std::function<void(uint32_t)>> PIP_ACTION_MAP {
    {ACTION_CLOSE, PictureInPictureManager::DoActionClose},
    {ACTION_REQUEST_CLOSE, PictureInPictureManager::DoActionCloseByRequest},
    {ACTION_PANEL_CLOSE, PictureInPictureManager::DoActionCloseByPanel},
    {ACTION_DUMPSTER_CLOSE, PictureInPictureManager::DoActionCloseByDumpster},
    {ACTION_PRE_RESTORE, PictureInPictureManager::DoPreRestore},
    {ACTION_RESTORE, PictureInPictureManager::DoRestore},
    {ACTION_PREPARE_SOURCE, PictureInPictureManager::DoPrepareSource},
    {ACTION_LOCATE_SOURCE, PictureInPictureManager::DoLocateSource},
    {ACTION_DESTROY, PictureInPictureManager::DoDestroy},
    {ACTION_BACKGROUND_AUTO_START, PictureInPictureManager::AutoStartPipWindow},
};

bool ShouldAutoStartDirectly(const sptr<PictureInPictureControllerBase>& pipController)
{
    if (pipController == nullptr) {
        return false;
    }
    return pipController->GetPiPNavigationId().empty() || pipController->IsTypeNodeEnabled();
}

NavigationController* GetNavControllerForAutoStart(const sptr<WindowSessionImpl>& mainWindow,
    const sptr<PictureInPictureControllerBase>& pipController)
{
    if (mainWindow == nullptr || pipController == nullptr) {
        return nullptr;
    }
    return NavigationController::GetNavigationController(mainWindow->GetUIContent(),
        pipController->GetPiPNavigationId());
}

}

std::map<int32_t, wptr<PictureInPictureControllerBase>> PictureInPictureManager::autoStartControllerMap_ = {};
std::map<uint32_t, std::vector<wptr<PictureInPictureControllerBase>>>
    PictureInPictureManager::mainWindowToAutoStartControllersMap_ = {};
std::map<uint32_t, sptr<PictureInPictureControllerBase>> PictureInPictureManager::windowToControllerMap_ = {};
std::mutex PictureInPictureManager::controllerMapMutex_;
std::shared_ptr<NativeReference> PictureInPictureManager::innerCallbackRef_ = nullptr;
PiPMultiConfig PictureInPictureManager::pipMultiConfig_ = GetDefaultPiPMultiConfig();

PictureInPictureManager::PictureInPictureManager()
{
}

PictureInPictureManager::~PictureInPictureManager()
{
}

bool PictureInPictureManager::IsSupportPiP()
{
    bool isSupportPiPFlag = false;
    SingletonContainer::Get<WindowAdapter>().GetIsPipEnabled(isSupportPiPFlag);
    return isSupportPiPFlag;
}

bool PictureInPictureManager::ShouldAbortPipStart()
{
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    for (auto& pair : windowToControllerMap_) {
        auto& controller = pair.second;
        if (controller != nullptr && controller->GetControllerState() == PiPWindowState::STATE_STARTING) {
            return true;
        }
    }
    return false;
}

void PictureInPictureManager::PutPipControllerInfo(uint32_t windowId, 
    sptr<PictureInPictureControllerBase> pipController)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId %{public}u", windowId);
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    auto iter = windowToControllerMap_.find(windowId);
    if (iter != windowToControllerMap_.end()) {
        UpdatePipGroupCountLocked(iter->second, false);
        iter->second = pipController;
    } else {
        windowToControllerMap_.insert(std::make_pair(windowId, pipController));
    }
    UpdatePipGroupCountLocked(pipController, true);
}

void PictureInPictureManager::RemovePipControllerInfo(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId %{public}u", windowId);
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    auto iter = windowToControllerMap_.find(windowId);
    if (iter != windowToControllerMap_.end()) {
        UpdatePipGroupCountLocked(iter->second, false);
        windowToControllerMap_.erase(iter);
    }
}

sptr<PictureInPictureControllerBase> PictureInPictureManager::GetPipControllerInfo(uint32_t windowId)
{
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    if (windowToControllerMap_.empty() || windowToControllerMap_.find(windowId) == windowToControllerMap_.end()) {
        TLOGE(WmsLogTag::WMS_PIP, "error, windowId: %{public}u not registered!", windowId);
        return nullptr;
    }
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u", windowId);
    return windowToControllerMap_[windowId];
}

bool PictureInPictureManager::HasActiveController()
{
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    for (auto& pair : windowToControllerMap_) {
        if (IsControllerStateActive(pair.second->GetControllerState())) {
            return true;
        }
    }
    return false;
}

bool PictureInPictureManager::IsActiveController(wptr<PictureInPictureControllerBase> pipController)
{
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    auto promoted = pipController.promote();
    if (promoted == nullptr) {
        return false;
    }
    for (auto& pair : windowToControllerMap_) {
        if (pair.second == promoted) {
            return IsControllerStateActive(pair.second->GetControllerState());
        }
    }
    return false;
}

bool PictureInPictureManager::IsControllerStateActive(PiPWindowState state)
{
    return state == PiPWindowState::STATE_STARTED ||
           state == PiPWindowState::STATE_STARTING ||
           state == PiPWindowState::STATE_RESTORING;
}

void PictureInPictureManager::AttachAutoStartController(int32_t handleId,
    wptr<PictureInPictureControllerBase> pipController)
{
    TLOGI(WmsLogTag::WMS_PIP, "handleId: %{public}d", handleId);
    if (pipController == nullptr) {
        return;
    }
    auto controller = pipController.promote();
    if (controller == nullptr) {
        return;
    }
    PiPGroupConfig group;
    auto type = static_cast<PiPTemplateType>(controller->GetPipTemplate());
    if (!pipMultiConfig_.FindGroupConfig(type, group)) {
        return;
    }
    uint32_t countInGroup = 0;
    sptr<PictureInPictureControllerBase> oldest = nullptr;
    for (const auto& [mainWindowId, controllers] : mainWindowToAutoStartControllersMap_) {
        for (const auto& wptrExisting : controllers) {
            auto existing = wptrExisting.promote();
            if (existing == nullptr) {
                continue;
            }
            PiPGroupConfig existingGroup;
            auto existingType = static_cast<PiPTemplateType>(existing->GetPipTemplate());
            if (!pipMultiConfig_.FindGroupConfig(existingType, existingGroup) ||
                existingGroup.groupId != group.groupId) {
                continue;
            }
            ++countInGroup;
            if (oldest == nullptr || existing->GetCreateTimestamp() < oldest->GetCreateTimestamp()) {
                oldest = existing;
            }
        }
    }
    if (group.maxCount > 0 && countInGroup >= group.maxCount && oldest != nullptr) {
        DetachAutoStartController(oldest->GetHandleId(), oldest);
    }
    autoStartControllerMap_[handleId] = pipController;
    auto it = mainWindowToAutoStartControllersMap_.find(controller->GetMainWindowId());
    if (it != mainWindowToAutoStartControllersMap_.end()) {
        for (const auto& wptr : it->second) {
            if (wptr.GetRefPtr() == pipController.GetRefPtr()) {
                TLOGI(WmsLogTag::WMS_PIP, "controller already registered for mainWindow: %{public}u",
                    controller->GetMainWindowId());
                return;
            }
        }
    }
    mainWindowToAutoStartControllersMap_[controller->GetMainWindowId()].push_back(pipController);
}

void PictureInPictureManager::DetachAutoStartController(int32_t handleId,
    wptr<PictureInPictureControllerBase> pipController)
{
    TLOGI(WmsLogTag::WMS_PIP, "handleId: %{public}d", handleId);
    autoStartControllerMap_.erase(handleId);
    auto controller = pipController.promote();
    if (controller != nullptr) {
        uint32_t mainWindowId = controller->GetMainWindowId();
        auto it = mainWindowToAutoStartControllersMap_.find(mainWindowId);
        if (it != mainWindowToAutoStartControllersMap_.end()) {
            auto& controllers = it->second;
            controllers.erase(std::remove_if(controllers.begin(), controllers.end(),
                [&pipController](const wptr<PictureInPictureControllerBase>& w) {
                    return w.GetRefPtr() == pipController.GetRefPtr();
                }),
                controllers.end());
            if (controllers.empty()) {
                mainWindowToAutoStartControllersMap_.erase(it);
            }
        }
    }
}

bool PictureInPictureManager::IsAttachedToSameWindow(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u", windowId);
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    for (auto& pair : windowToControllerMap_) {
        auto& controller = pair.second;
        if (controller != nullptr && controller->GetMainWindowId() == windowId) {
            return true;
        }
    }
    return false;
}

sptr<Window> PictureInPictureManager::GetCurrentWindowByMainWindowId(uint32_t windowId)
{
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    for (auto& pair : windowToControllerMap_) {
        auto& controller = pair.second;
        if (controller != nullptr && controller->GetMainWindowId() == windowId) {
            return controller->GetPipWindow();
        }
    }
    return nullptr;
}

sptr<Window> PictureInPictureManager::GetSameGroupWindowByMainWindowId(uint32_t windowId, 
    PiPTemplateType PipType)
{
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    PiPGroupConfig group;
    if (!pipMultiConfig_.FindGroupConfig(PipType, group)) {
        return nullptr;
    }
    auto firstController = static_cast<sptr<PictureInPictureControllerBase>>(nullptr);
    for (auto& pair : windowToControllerMap_) {
        auto& controller = pair.second;
        auto type = static_cast<PiPTemplateType>(controller->GetPipTemplate());
        if (controller != nullptr && controller->GetMainWindowId() == windowId &&
            std::find(group.types.begin(), group.types.end(), type) != group.types.end()) {
            if (firstController == nullptr || firstController->GetStartTimestamp() > controller->GetStartTimestamp()) {
                firstController = controller;
            }
        }
    }
    if (firstController != nullptr) {
        return firstController->GetPipWindow();
    }
    return nullptr;
}

sptr<Window> PictureInPictureManager::GetCurrentWindow()
{
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    if (!windowToControllerMap_.empty() && windowToControllerMap_.begin()->second != nullptr) {
        return windowToControllerMap_.begin()->second->GetPipWindow();
    }
    return nullptr;
}

void PictureInPictureManager::DoPreRestore(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u", windowId);
    if (auto controller = GetPipControllerInfo(windowId)) {
        controller->PreRestorePictureInPicture();
    }
}

void PictureInPictureManager::DoRestore(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u", windowId);
    if (auto controller = GetPipControllerInfo(windowId)) {
        controller->RestorePictureInPictureWindow();
    }
}

void PictureInPictureManager::DoPrepareSource(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u", windowId);
    if (auto controller = GetPipControllerInfo(windowId)) {
        controller->PrepareSource();
    }
}

void PictureInPictureManager::DoLocateSource(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u", windowId);
    if (auto controller = GetPipControllerInfo(windowId)) {
        controller->LocateSource();
    }
}

void PictureInPictureManager::DoClose(uint32_t windowId, bool destroyWindow, bool byPriority)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u, destroyWindow:%{public}d, byPriority:%{public}d",
        windowId, destroyWindow, byPriority);
    if (auto controller = GetPipControllerInfo(windowId)) {
        StopPipType currentStopType = StopPipType::NULL_STOP;
        if (!byPriority) {
            currentStopType = StopPipType::USER_STOP;
        } else {
            currentStopType = StopPipType::OTHER_PACKAGE_STOP;
        }
        controller->SetStateChangeReason(PiPStateChangeReason::OTHER);
        controller->StopPictureInPicture(destroyWindow, currentStopType, !byPriority);
    }
}

void PictureInPictureManager::DoCloseWithReason(uint32_t windowId, bool destroyWindow, bool byPriority,
    PiPStateChangeReason reason)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u, destroyWindow:%{public}d, byPriority:%{public}d",
        windowId, destroyWindow, byPriority);
    if (auto controller = GetPipControllerInfo(windowId)) {
        StopPipType currentStopType = StopPipType::NULL_STOP;
        if (!byPriority) {
            currentStopType = StopPipType::USER_STOP;
        } else {
            currentStopType = StopPipType::OTHER_PACKAGE_STOP;
        }
        controller->SetStateChangeReason(reason);
        controller->StopPictureInPicture(destroyWindow, currentStopType, !byPriority);
    }
}

void PictureInPictureManager::DoActionClose(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u", windowId);
    DoClose(windowId, true, false);
}

void PictureInPictureManager::DoActionCloseByRequest(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u", windowId);
    DoCloseWithReason(windowId, true, false, PiPStateChangeReason::REQUEST_DELETE);
}

void PictureInPictureManager::DoActionCloseByPanel(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u", windowId);
    DoCloseWithReason(windowId, true, false, PiPStateChangeReason::PANEL_ACTION_DELETE);
}

void PictureInPictureManager::DoActionCloseByDumpster(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u", windowId);
    DoCloseWithReason(windowId, true, false, PiPStateChangeReason::DRAG_DELETE);
}

void PictureInPictureManager::DoDestroy(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u", windowId);
    if (auto controller = GetPipControllerInfo(windowId)) {
        controller->DestroyPictureInPictureWindow();
    }
}

void PictureInPictureManager::DoActionEvent(uint32_t windowId, const std::string& actionName, int32_t status)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u, actionName: %{public}s", windowId, actionName.c_str());
    auto func = PIP_ACTION_MAP.find(actionName);
    if (func == PIP_ACTION_MAP.end()) {
        TLOGI(WmsLogTag::WMS_PIP, "through pass");
        if (auto controller = GetPipControllerInfo(windowId)) {
            controller->DoActionEvent(actionName, status);
        }
        return;
    }
    func->second(windowId);
}

void PictureInPictureManager::DoControlEvent(uint32_t windowId, PiPControlType controlType, PiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u, controlType:%{public}u, status:%{public}d",
        windowId, controlType, status);
    if (auto controller = GetPipControllerInfo(windowId)) {
        controller->DoControlEvent(controlType, status);
    }
}

void PictureInPictureManager::AutoStartPipWindow(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "mainWindowId: %{public}u", windowId);
    auto it = mainWindowToAutoStartControllersMap_.find(windowId);
    if (it == mainWindowToAutoStartControllersMap_.end() || it->second.empty()) {
        TLOGE(WmsLogTag::WMS_PIP, "no autoStartController registered for mainWindow: %{public}u", windowId);
        return;
    }
    for (const auto& wptrPipController : it->second) {
        auto pipController = wptrPipController.promote();
        if (pipController == nullptr) {
            continue;
        }
        pipController->SetStateChangeReason(PiPStateChangeReason::AUTO_START);
        if (ShouldAutoStartDirectly(pipController)) {
            TLOGI(WmsLogTag::WMS_PIP, "No use navigation for auto start");
            pipController->StartPictureInPicture(StartPipType::AUTO_START);
            continue;
        }
        sptr<WindowSessionImpl> mainWindow =
            WindowSceneSessionImpl::GetMainWindowWithId(pipController->GetMainWindowId());
        auto navController = GetNavControllerForAutoStart(mainWindow, pipController);
        if (!navController) {
            TLOGE(WmsLogTag::WMS_PIP, "navController is nullptr");
            continue;
        }
        if (!navController->IsNavDestinationInTopStack()) {
            TLOGE(WmsLogTag::WMS_PIP, "Top is not navDestination");
            continue;
        }
        int32_t handleId = navController->GetTopHandle();
        auto mapIt = autoStartControllerMap_.find(handleId);
        if (mapIt == autoStartControllerMap_.end()) {
            TLOGE(WmsLogTag::WMS_PIP, "GetNavController info error, %{public}d not registered", handleId);
            continue;
        }
        auto handlePipController = mapIt->second.promote();
        if (handlePipController == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "GetNavController info error, %{public}d not registered", handleId);
            continue;
        }
        handlePipController->StartPictureInPicture(StartPipType::AUTO_START);
    }
}

void PictureInPictureManager::PipSizeChange(uint32_t windowId, double width, double height, double scale)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u", windowId);
    if (auto controller = GetPipControllerInfo(windowId)) {
        controller->PipSizeChange(width, height, scale);
    }
}

void PictureInPictureManager::DoActiveStatusChangeEvent(uint32_t windowId, bool status)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u, status: %{public}d", windowId, status);
    if (auto controller = GetPipControllerInfo(windowId)) {
        controller->ActiveStatusChange(status);
    }
}

bool PictureInPictureManager::GetPipEnabled()
{
    bool isPipEnabled = false;
    SingletonContainer::Get<WindowAdapter>().GetIsPipEnabled(isPipEnabled);
    return isPipEnabled;
}

void PictureInPictureManager::SetMultiPipConfig(const PiPMultiConfig& config)
{
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    pipMultiConfig_ = config;
    pipMultiConfig_.ResetGroupCounts();
    for (const auto& pair : windowToControllerMap_) {
        UpdatePipGroupCountLocked(pair.second, true);
    }
}

const PiPMultiConfig& PictureInPictureManager::GetMultiPipConfig()
{
    return pipMultiConfig_;
}

bool PictureInPictureManager::IsTemplateTypeSupported(PiPTemplateType type)
{
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    PiPGroupConfig group;
    return pipMultiConfig_.FindGroupConfig(type, group);
}

bool PictureInPictureManager::IsPipGroupLimitReached(PiPTemplateType type)
{
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    PiPGroupConfig group;
    if (!pipMultiConfig_.FindGroupConfig(type, group)) {
        TLOGE(WmsLogTag::WMS_PIP, "no group config for type %{public}u", static_cast<uint32_t>(type));
        return false;
    }
    return pipMultiConfig_.GetGroupCount(group.groupId) >= group.maxCount;
}

void PictureInPictureManager::UpdatePipGroupCountLocked(const sptr<PictureInPictureControllerBase>& controller,
    bool increase)
{
    if (controller == nullptr) {
        return;
    }
    auto type = static_cast<PiPTemplateType>(controller->GetPipTemplate());
    PiPGroupConfig group;
    if (!pipMultiConfig_.FindGroupConfig(type, group)) {
        return;
    }
    if (increase) {
        pipMultiConfig_.IncreaseGroupCount(group.groupId);
    } else {
        pipMultiConfig_.DecreaseGroupCount(group.groupId);
    }
}
} // namespace Rosen
} // namespace OHOS