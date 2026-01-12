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

#include "parameters.h"
#include "picture_in_picture_controller.h"
#include "window_adapter.h"
#include "window_manager_hilog.h"
#include "window_scene_session_impl.h"

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

const std::map<std::string, std::function<void()>> PIP_ACTION_MAP {
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
}

sptr<PictureInPictureControllerBase> PictureInPictureManager::activeController_ = nullptr;
wptr<PictureInPictureController> PictureInPictureManager::autoStartController_ = nullptr;
std::map<int32_t, wptr<PictureInPictureController>> PictureInPictureManager::autoStartControllerMap_ = {};
std::map<int32_t, sptr<PictureInPictureControllerBase>> PictureInPictureManager::windowToControllerMap_ = {};
std::shared_ptr<NativeReference> PictureInPictureManager::innerCallbackRef_ = nullptr;

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
    return activeController_ != nullptr && activeController_->GetControllerState() == PiPWindowState::STATE_STARTING;
}

void PictureInPictureManager::PutPipControllerInfo(int32_t windowId, sptr<PictureInPictureControllerBase> pipController)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId %{public}u", windowId);
    windowToControllerMap_.insert(std::make_pair(windowId, pipController));
}

void PictureInPictureManager::RemovePipControllerInfo(int32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId %{public}u", windowId);
    windowToControllerMap_.erase(windowId);
}

sptr<PictureInPictureControllerBase> PictureInPictureManager::GetPipControllerInfo(int32_t windowId)
{
    if (windowToControllerMap_.empty() || windowToControllerMap_.find(windowId) == windowToControllerMap_.end()) {
        TLOGE(WmsLogTag::WMS_PIP, "error, windowId: %{public}d not registered!", windowId);
        return nullptr;
    }
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}d", windowId);
    return windowToControllerMap_[windowId];
}

bool PictureInPictureManager::HasActiveController()
{
    return activeController_ != nullptr;
}

bool PictureInPictureManager::IsActiveController(wptr<PictureInPictureControllerBase> pipController)
{
    if (!HasActiveController()) {
        return false;
    }
    bool res = pipController.GetRefPtr() == activeController_.GetRefPtr();
    TLOGD(WmsLogTag::WMS_PIP, "res: %{public}u", res);
    return res;
}

void PictureInPictureManager::SetActiveController(sptr<PictureInPictureControllerBase> pipController)
{
    TLOGD(WmsLogTag::WMS_PIP, "in");
    activeController_ = pipController;
}

void PictureInPictureManager::RemoveActiveController(wptr<PictureInPictureControllerBase> pipController)
{
    TLOGD(WmsLogTag::WMS_PIP, "in");
    if (HasActiveController() && pipController.GetRefPtr() == activeController_.GetRefPtr()) {
        activeController_ = nullptr;
    }
}

void PictureInPictureManager::AttachAutoStartController(int32_t handleId,
    wptr<PictureInPictureController> pipController)
{
    TLOGD(WmsLogTag::WMS_PIP, "handleId: %{public}u", handleId);
    if (pipController == nullptr) {
        return;
    }
    autoStartController_ = pipController;
    autoStartControllerMap_[handleId] = pipController;
}

void PictureInPictureManager::DetachAutoStartController(int32_t handleId,
    wptr<PictureInPictureController> pipController)
{
    TLOGI(WmsLogTag::WMS_PIP, "handleId: %{public}u", handleId);
    if (autoStartController_ == nullptr) {
        TLOGW(WmsLogTag::WMS_PIP, "detach failed as none autoStartController exist");
        return;
    }
    if (pipController != nullptr && pipController.GetRefPtr() != autoStartController_.GetRefPtr()) {
        TLOGW(WmsLogTag::WMS_PIP, "not same pip controller");
        return;
    }
    autoStartController_ = nullptr;
    autoStartControllerMap_.erase(handleId);
}

bool PictureInPictureManager::IsAttachedToSameWindow(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "windowId: %{public}u", windowId);
    if (!HasActiveController()) {
        return false;
    }
    return activeController_->GetMainWindowId() == windowId;
}

sptr<Window> PictureInPictureManager::GetCurrentWindow()
{
    if (!HasActiveController()) {
        return nullptr;
    }
    return activeController_->GetPipWindow();
}

void PictureInPictureManager::DoPreRestore()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (auto controller = GetActiveController()) {
        controller->PreRestorePictureInPicture();
    }
}

void PictureInPictureManager::DoRestore()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (auto controller = GetActiveController()) {
        controller->RestorePictureInPictureWindow();
    }
}

void PictureInPictureManager::DoPrepareSource()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (auto controller = GetActiveController()) {
        controller->PrepareSource();
    }
}

void PictureInPictureManager::DoLocateSource()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (auto controller = GetActiveController()) {
        controller->LocateSource();
    }
}

void PictureInPictureManager::DoClose(bool destroyWindow, bool byPriority)
{
    TLOGI(WmsLogTag::WMS_PIP, "destroyWindow:%{public}d, byPriority:%{public}d", destroyWindow, byPriority);
    if (auto controller = GetActiveController()) {
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

void PictureInPictureManager::DoCloseWithReason(bool destroyWindow, bool byPriority, PiPStateChangeReason reason)
{
    TLOGI(WmsLogTag::WMS_PIP, "destroyWindow:%{public}d, byPriority:%{public}d", destroyWindow, byPriority);
    if (auto controller = GetActiveController()) {
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

void PictureInPictureManager::DoActionClose()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    DoClose(true, false);
}

void PictureInPictureManager::DoActionCloseByRequest()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    DoCloseWithReason(true, false, PiPStateChangeReason::REQUEST_DELETE);
}
 
void PictureInPictureManager::DoActionCloseByPanel()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    DoCloseWithReason(true, false, PiPStateChangeReason::PANEL_ACTION_DELETE);
}
 
void PictureInPictureManager::DoActionCloseByDumpster()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    DoCloseWithReason(true, false, PiPStateChangeReason::DRAG_DELETE);
}

void PictureInPictureManager::DoDestroy()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (auto controller = GetActiveController()) {
        controller->DestroyPictureInPictureWindow();
    }
}

void PictureInPictureManager::DoActionEvent(const std::string& actionName, int32_t status)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    auto func = PIP_ACTION_MAP.find(actionName);
    if (func == PIP_ACTION_MAP.end()) {
        TLOGI(WmsLogTag::WMS_PIP, "through pass");
        if (auto controller = GetActiveController()) {
            controller->DoActionEvent(actionName, status);
        }
        return;
    }
    func->second();
}

void PictureInPictureManager::DoControlEvent(PiPControlType controlType, PiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType:%{public}u, enabled:%{public}d", controlType, status);
    if (auto controller = GetActiveController()) {
        controller->DoControlEvent(controlType, status);
    }
}

void PictureInPictureManager::AutoStartPipWindow()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (autoStartController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "autoStartController_ is null");
        return;
    }
    auto autoStartController = autoStartController_.promote();
    if (autoStartController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "autoStartController is null");
        return;
    }
    autoStartController->SetStateChangeReason(PiPStateChangeReason::AUTO_START);
    if (autoStartController->GetPiPNavigationId().empty() || autoStartController->IsTypeNodeEnabled()) {
        TLOGI(WmsLogTag::WMS_PIP, "No use navigation for auto start");
        autoStartController->StartPictureInPicture(StartPipType::AUTO_START);
        return;
    }
    sptr<WindowSessionImpl> mainWindow = WindowSceneSessionImpl::GetMainWindowWithId(
        autoStartController->GetMainWindowId());
    if (mainWindow) {
        auto navController = NavigationController::GetNavigationController(mainWindow->GetUIContent(),
            autoStartController->GetPiPNavigationId());
        if (!navController) {
            TLOGE(WmsLogTag::WMS_PIP, "navController is nullptr");
            return;
        }
        if (navController->IsNavDestinationInTopStack()) {
            int handleId = navController->GetTopHandle();
            if (autoStartControllerMap_.empty() ||
                autoStartControllerMap_.find(handleId) == autoStartControllerMap_.end()) {
                TLOGE(WmsLogTag::WMS_PIP, "GetNavController info error, %{public}d not registered", handleId);
                return;
            }
            auto wptrPipController = autoStartControllerMap_[handleId];
            if (wptrPipController == nullptr) {
                TLOGE(WmsLogTag::WMS_PIP, "wptrPipController is nullptr");
                return;
            }
            auto pipController = wptrPipController.promote();
            if (!pipController) {
                TLOGE(WmsLogTag::WMS_PIP, "pipController is nullptr");
                return;
            }
            pipController->StartPictureInPicture(StartPipType::AUTO_START);
        } else {
            TLOGE(WmsLogTag::WMS_PIP, "Top is not navDestination");
        }
    }
}

void PictureInPictureManager::PipSizeChange(double width, double height, double scale)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (auto controller = GetActiveController()) {
        controller->PipSizeChange(width, height, scale);
    }
}

void PictureInPictureManager::DoActiveStatusChangeEvent(bool status)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (auto controller = GetActiveController()) {
        controller->ActiveStatusChange(status);
    }
}

bool PictureInPictureManager::GetPipEnabled()
{
    bool isPipEnabled = false;
    SingletonContainer::Get<WindowAdapter>().GetIsPipEnabled(isPipEnabled);
    return isPipEnabled;
}

}
}
