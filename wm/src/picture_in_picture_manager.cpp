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

#include <refbase.h>
#include "picture_in_picture_controller.h"
#include "window.h"
#include "window_manager_hilog.h"
#include "window_scene_session_impl.h"
#include "wm_common.h"
#include "scene_board_judgement.h"

namespace OHOS {
namespace Rosen {
namespace {
    const std::string ACTION_CLOSE = "close";
    const std::string ACTION_PRE_RESTORE = "pre_restore";
    const std::string ACTION_RESTORE = "restore";
    const std::string ACTION_DESTROY = "destroy";
    const std::string ACTION_PREPARE_SOURCE = "prepare_source";
    const std::string ACTION_LOCATE_SOURCE = "locate_source";
    const std::string ACTION_BACKGROUND_AUTO_START = "background_auto_start";

    const std::map<std::string, std::function<void()>> PIP_ACTION_MAP {
        {ACTION_CLOSE, PictureInPictureManager::DoActionClose},
        {ACTION_PRE_RESTORE, PictureInPictureManager::DoPreRestore},
        {ACTION_RESTORE, PictureInPictureManager::DoRestore},
        {ACTION_PREPARE_SOURCE, PictureInPictureManager::DoPrepareSource},
        {ACTION_LOCATE_SOURCE, PictureInPictureManager::DoLocateSource},
        {ACTION_DESTROY, PictureInPictureManager::DoDestroy},
        {ACTION_BACKGROUND_AUTO_START, PictureInPictureManager::AutoStartPipWindow},
    };
}

sptr<PictureInPictureController> PictureInPictureManager::activeController_ = nullptr;
wptr<PictureInPictureController> PictureInPictureManager::autoStartController_ = nullptr;
std::map<int32_t, wptr<PictureInPictureController>> PictureInPictureManager::autoStartControllerMap_ = {};
std::map<int32_t, sptr<PictureInPictureController>> PictureInPictureManager::windowToControllerMap_ = {};
std::shared_mutex PictureInPictureManager::controllerMapMutex_;
std::mutex PictureInPictureManager::mutex_;
std::shared_ptr<NativeReference> PictureInPictureManager::innerCallbackRef_ = nullptr;

PictureInPictureManager::PictureInPictureManager()
{
}

PictureInPictureManager::~PictureInPictureManager()
{
}

bool PictureInPictureManager::IsSupportPiP()
{
    return SceneBoardJudgement::IsSceneBoardEnabled();
}

bool PictureInPictureManager::ShouldAbortPipStart()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return activeController_ != nullptr && activeController_->GetControllerState() == PiPWindowState::STATE_STARTING;
}

void PictureInPictureManager::PutPipControllerInfo(int32_t windowId, sptr<PictureInPictureController> pipController)
{
    TLOGD(WmsLogTag::WMS_PIP, "PutPipControllerInfo called, windowId %{public}u", windowId);
    std::unique_lock<std::shared_mutex> lock(controllerMapMutex_);
    windowToControllerMap_.insert(std::make_pair(windowId, pipController));
}

void PictureInPictureManager::RemovePipControllerInfo(int32_t windowId)
{
    TLOGD(WmsLogTag::WMS_PIP, "called, windowId %{public}u", windowId);
    std::unique_lock<std::shared_mutex> lock(controllerMapMutex_);
    windowToControllerMap_.erase(windowId);
}

sptr<PictureInPictureController> PictureInPictureManager::GetPipControllerInfo(int32_t windowId)
{
    std::shared_lock<std::shared_mutex> lock(controllerMapMutex_);
    if (windowToControllerMap_.empty() || windowToControllerMap_.find(windowId) == windowToControllerMap_.end()) {
        TLOGE(WmsLogTag::WMS_PIP, "GetPipControllerInfo error, %{public}d not registered!", windowId);
        return nullptr;
    }
    TLOGI(WmsLogTag::WMS_PIP, "windowId:%{public}d", windowId);
    return windowToControllerMap_[windowId];
}

bool PictureInPictureManager::HasActiveController()
{
    return activeController_ != nullptr;
}

bool PictureInPictureManager::IsActiveController(wptr<PictureInPictureController> pipController)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!HasActiveController()) {
        return false;
    }
    bool res = pipController.GetRefPtr() == activeController_.GetRefPtr();
    TLOGD(WmsLogTag::WMS_PIP, "IsActiveController %{public}u", res);
    return res;
}

void PictureInPictureManager::SetActiveController(sptr<PictureInPictureController> pipController)
{
    TLOGD(WmsLogTag::WMS_PIP, "called");
    std::lock_guard<std::mutex> lock(mutex_);
    activeController_ = pipController;
}

void PictureInPictureManager::RemoveActiveController(wptr<PictureInPictureController> pipController)
{
    TLOGD(WmsLogTag::WMS_PIP, "called");
    if (HasActiveController() && pipController.GetRefPtr() == activeController_.GetRefPtr()) {
        activeController_ = nullptr;
    }
}

void PictureInPictureManager::AttachAutoStartController(int32_t handleId,
    wptr<PictureInPictureController> pipController)
{
    TLOGD(WmsLogTag::WMS_PIP, "AttachAutoStartController, %{public}u", handleId);
    if (pipController == nullptr) {
        return;
    }
    autoStartController_ = pipController;
    autoStartControllerMap_[handleId] = pipController;
}

void PictureInPictureManager::DetachAutoStartController(int32_t handleId,
    wptr<PictureInPictureController> pipController)
{
    TLOGD(WmsLogTag::WMS_PIP, "Detach active pipController, %{public}u", handleId);
    autoStartControllerMap_.erase(handleId);
    if (autoStartController_ == nullptr) {
        return;
    }
    if (pipController != nullptr &&
        pipController.GetRefPtr() != autoStartController_.GetRefPtr()) {
        TLOGE(WmsLogTag::WMS_PIP, "not same pip controller or no active pip controller");
        return;
    }
    autoStartController_ = nullptr;
}

bool PictureInPictureManager::IsAttachedToSameWindow(uint32_t windowId)
{
    TLOGD(WmsLogTag::WMS_PIP, "called %{public}u", windowId);
    std::lock_guard<std::mutex> lock(mutex_);
    if (!HasActiveController()) {
        return false;
    }
    return activeController_->GetMainWindowId() == windowId;
}

sptr<Window> PictureInPictureManager::GetCurrentWindow()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!HasActiveController()) {
        return nullptr;
    }
    return activeController_->GetPipWindow();
}

void PictureInPictureManager::DoPreRestore()
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    std::lock_guard<std::mutex> lock(mutex_);
    if (!HasActiveController()) {
        return;
    }
    activeController_->PreRestorePictureInPicture();
}

void PictureInPictureManager::DoRestore()
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    std::lock_guard<std::mutex> lock(mutex_);
    if (!HasActiveController()) {
        return;
    }
    activeController_->RestorePictureInPictureWindow();
}

void PictureInPictureManager::DoPrepareSource()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    std::lock_guard<std::mutex> lock(mutex_);
    if (!HasActiveController()) {
        return;
    }
    activeController_->PrepareSource();
}

void PictureInPictureManager::DoLocateSource()
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    std::lock_guard<std::mutex> lock(mutex_);
    if (!HasActiveController()) {
        return;
    }
    activeController_->LocateSource();
}

void PictureInPictureManager::DoClose(bool destroyWindow, bool byPriority)
{
    TLOGD(WmsLogTag::WMS_PIP, "called");
    std::lock_guard<std::mutex> lock(mutex_);
    if (!HasActiveController()) {
        return;
    }
    StopPipType currentStopType = StopPipType::NULL_STOP;
    if (!byPriority) {
        currentStopType = StopPipType::USER_STOP;
    } else {
        currentStopType = StopPipType::OTHER_PACKAGE_STOP;
    }
    activeController_->StopPictureInPicture(destroyWindow, currentStopType, !byPriority);
}

void PictureInPictureManager::DoActionClose()
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    DoClose(true, false);
}

void PictureInPictureManager::DoDestroy()
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    if (!HasActiveController()) {
        return;
    }
    activeController_->DestroyPictureInPictureWindow();
}

void PictureInPictureManager::DoActionEvent(const std::string& actionName, int32_t status)
{
    TLOGD(WmsLogTag::WMS_PIP, "called");
    auto func = PIP_ACTION_MAP.find(actionName);
    if (func == PIP_ACTION_MAP.end()) {
        TLOGI(WmsLogTag::WMS_PIP, "through pass");
        std::lock_guard<std::mutex> lock(mutex_);
        if (HasActiveController()) {
            activeController_->DoActionEvent(actionName, status);
        }
        return;
    }
    func->second();
}

void PictureInPictureManager::DoControlEvent(PiPControlType controlType, PiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType:%{public}u, enabled:%{public}d", controlType, status);
    std::lock_guard<std::mutex> lock(mutex_);
    if (!HasActiveController()) {
        return;
    }
    activeController_->DoControlEvent(controlType, status);
}

void PictureInPictureManager::AutoStartPipWindow()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (autoStartController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "autoStartController_ is null");
        return;
    }
    if (autoStartController_->GetPiPNavigationId().empty() || autoStartController_->IsTypeNodeEnabled()) {
        TLOGI(WmsLogTag::WMS_PIP, "No use navigation for auto start");
        autoStartController_->StartPictureInPicture(StartPipType::AUTO_START);
        return;
    }
    sptr<WindowSessionImpl> mainWindow = WindowSceneSessionImpl::GetMainWindowWithId(
        autoStartController_->GetMainWindowId());
    if (mainWindow) {
        auto navController = NavigationController::GetNavigationController(mainWindow->GetUIContent(),
            autoStartController_->GetPiPNavigationId());
        if (!navController) {
            TLOGE(WmsLogTag::WMS_PIP, "navController is nullptr");
            return;
        }
        if (navController->IsNavDestinationInTopStack()) {
            int handleId = navController->GetTopHandle();
            if (autoStartControllerMap_.empty() ||
                autoStartControllerMap_.find(handleId) == autoStartControllerMap_.end()) {
                TLOGE(WmsLogTag::WMS_PIP, "GetNAvController info error, %{public}d not registered", handleId);
                return;
            }
            auto pipController = autoStartControllerMap_[handleId];
            pipController->StartPictureInPicture(StartPipType::AUTO_START);
        } else {
            TLOGE(WmsLogTag::WMS_PIP, "Top is not navDestination");
        }
    }
}
}
}
