/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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
#include "float_view_manager.h"

#include "window.h"
#include "window_manager_hilog.h"
#include "float_window_manager.h"
 
namespace OHOS {
namespace Rosen {
namespace {
const std::string ACTION_START = "start";
const std::string ACTION_CLOSE = "close";
const std::string ACTION_HIDE = "hide";
const std::string ACTION_IN_SIDEBAR = "inSidebar";
const std::string ACTION_IN_FLOATING_BALL = "inFloatingBall";

const std::map<std::string, std::function<void(uint32_t, const std::string&)>> FV_ACTION_MAP {
    {ACTION_START, FloatViewManager::DoActionStart},
    {ACTION_CLOSE, FloatViewManager::DoActionClose},
    {ACTION_HIDE, FloatViewManager::DoActionHide},
    {ACTION_IN_SIDEBAR, FloatViewManager::DoActionInSidebar},
    {ACTION_IN_FLOATING_BALL, FloatViewManager::DoActionInFloatingBall},
};
}

bool FloatViewManager::isSupportFloatView_ = false;
sptr<FloatViewController> FloatViewManager::activeController_ = nullptr;
std::mutex FloatViewManager::controllerMapMutex_;
std::map<uint32_t, wptr<FloatViewController>> FloatViewManager::windowId2Controller_ = {};

bool FloatViewManager::HasActiveController()
{
    return activeController_ != nullptr;
}

bool FloatViewManager::IsActiveController(const wptr<FloatViewController>& fvControllerWeak)
{
    if (!HasActiveController()) {
        return false;
    }
    bool res = fvControllerWeak.GetRefPtr() == activeController_.GetRefPtr();
    return res;
}

void FloatViewManager::SetActiveController(const wptr<FloatViewController>& fvControllerWeak)
{
    auto fvController = fvControllerWeak.promote();
    if (fvController == nullptr) {
        return;
    }
    activeController_ = fvController;
}

void FloatViewManager::RemoveActiveController(const wptr<FloatViewController>& fvControllerWeak)
{
    if (HasActiveController() && fvControllerWeak.GetRefPtr() == activeController_.GetRefPtr()) {
        activeController_ = nullptr;
    }
}

void FloatViewManager::AddController(uint32_t windowId, wptr<FloatViewController> controller)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "add fv controller, windowId: %{public}u", windowId);
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    windowId2Controller_.insert(std::make_pair(windowId, controller));
}

void FloatViewManager::RemoveController(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "remove fv controller, windowId: %{public}u", windowId);
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    auto iter = windowId2Controller_.find(windowId);
    if (iter != windowId2Controller_.end()) {
        windowId2Controller_.erase(iter);
    }
}

wptr<FloatViewController> FloatViewManager::GetController(uint32_t windowId)
{
    std::lock_guard<std::mutex> lock(controllerMapMutex_);
    auto iter = windowId2Controller_.find(windowId);
    if (iter != windowId2Controller_.end()) {
        return iter->second;
    }
    return nullptr;
}

void FloatViewManager::DoActionEvent(uint32_t windowId, const std::string& actionName, const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "in, actionName %{public}s, reason: %{public}s", actionName.c_str(), reason.c_str());
    auto func = FV_ACTION_MAP.find(actionName);
    if (func == FV_ACTION_MAP.end()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "no func to process");
        return;
    }
    func->second(windowId, reason);
}

void FloatViewManager::DoActionStart(uint32_t windowId, const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "start in");
    if (auto controller = GetController(windowId).promote()) {
        controller->ChangeState(FvWindowState::FV_STATE_STARTED);
        if (controller->IsBind()) {
            FloatWindowManager::ProcessBindFloatViewStateChange(controller, FvWindowState::FV_STATE_STARTED);
        }
    }
}

void FloatViewManager::DoActionCloseByMainWindow(uint32_t mainWindowId, const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "close in, mainWindowId: %{public}u, reason: %{public}s",
        mainWindowId, reason.c_str());
    std::vector<uint32_t> controllerList;
    {
        std::lock_guard<std::mutex> lock(controllerMapMutex_);
        for (const auto& pair : windowId2Controller_) {
            auto controller = pair.second.promote();
            if (controller != nullptr && controller->GetMainWindowId() == mainWindowId) {
                controllerList.push_back(pair.first);
            }
        }
    }
    for (const auto& windowId : controllerList) {
        DoActionClose(windowId, reason);
    }
}

void FloatViewManager::DoActionClose(uint32_t windowId, const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "close in, reason: %{public}s", reason.c_str());
    if (auto controller = GetController(windowId).promote()) {
        controller->StopFloatView(reason);
    }
}

void FloatViewManager::DoActionHide(uint32_t windowId, const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "hide in");
    if (auto controller = GetController(windowId).promote()) {
        controller->ChangeState(FvWindowState::FV_STATE_HIDDEN);
    }
}

void FloatViewManager::DoActionInSidebar(uint32_t windowId, const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "inSidebar in");
    if (auto controller = GetController(windowId).promote()) {
        controller->ChangeState(FvWindowState::FV_STATE_IN_SIDEBAR);
    }
}

void FloatViewManager::DoActionInFloatingBall(uint32_t windowId, const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "inFloatingBall in");
    if (auto controller = GetController(windowId).promote()) {
        controller->ChangeState(FvWindowState::FV_STATE_IN_FLOATING_BALL);
        if (controller->IsBind()) {
            FloatWindowManager::ProcessBindFloatViewStateChange(controller, FvWindowState::FV_STATE_IN_FLOATING_BALL);
        }
    }
}

void FloatViewManager::SyncFvWindowInfo(uint32_t windowId, const FloatViewWindowInfo& windowInfo,
    const std::string& reason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "SyncFvWindowInfo in, reason: %{public}s", reason.c_str());
    if (auto controller = GetController(windowId).promote()) {
        controller->SyncWindowInfo(windowId, windowInfo, reason);
    }
}

void FloatViewManager::SyncFvLimits(uint32_t windowId, const std::map<uint32_t, FloatViewLimits>& limits)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "SyncFvLimits in");
    if (auto controller = GetController(windowId).promote()) {
        controller->SyncLimits(windowId, limits);
    }
}
} // namespace Rosen
} // namespace OHOS
