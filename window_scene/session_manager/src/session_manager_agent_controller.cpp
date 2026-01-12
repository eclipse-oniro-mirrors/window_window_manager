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

#include "session_manager_agent_controller.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "SessionManagerAgentController"};
}
WM_IMPLEMENT_SINGLE_INSTANCE(SessionManagerAgentController)

WMError SessionManagerAgentController::RegisterWindowManagerAgent(const sptr<IWindowManagerAgent>& windowManagerAgent,
    WindowManagerAgentType type, int32_t pid)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "type: %{public}u", static_cast<uint32_t>(type));
    if (smAgentContainer_.RegisterAgent(windowManagerAgent, type)) {
        std::lock_guard<std::mutex> lock(windowManagerAgentPidMapMutex_);
        auto it = windowManagerPidAgentMap_.find(pid);
        if (it != windowManagerPidAgentMap_.end()) {
            auto& typeAgentMap = it->second;
            auto typeAgentIter = typeAgentMap.find(type);
            if (typeAgentIter != typeAgentMap.end()) {
                smAgentContainer_.UnregisterAgent(typeAgentIter->second, type);
                windowManagerAgentPairMap_.erase((typeAgentIter->second)->AsObject());
            }
            typeAgentMap.insert(std::map<WindowManagerAgentType,
                sptr<IWindowManagerAgent>>::value_type(type, windowManagerAgent));
        } else {
            std::map<WindowManagerAgentType, sptr<IWindowManagerAgent>> typeAgentMap;
            typeAgentMap.insert(std::map<WindowManagerAgentType,
                sptr<IWindowManagerAgent>>::value_type(type, windowManagerAgent));
            TLOGI(WmsLogTag::WMS_MAIN, "insert pid: %{public}d, type: %{public}u",
                pid, static_cast<uint32_t>(type));
            windowManagerPidAgentMap_.insert(std::map<int32_t,
                std::map<WindowManagerAgentType, sptr<IWindowManagerAgent>>>::value_type(pid, typeAgentMap));
        }
        std::pair<int32_t, WindowManagerAgentType> pidPair = {pid, type};
        windowManagerAgentPairMap_.insert(std::map<sptr<IRemoteObject>,
            std::pair<int32_t, WindowManagerAgentType>>::value_type(windowManagerAgent->AsObject(), pidPair));
        if (type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_STYLE &&
            windowManagementMode_ != WindowManagementMode::UNDEFINED) {
            NotifyWindowStyleChange(windowManagementMode_ == WindowManagementMode::FREEFORM ?
                WindowStyleType::WINDOW_STYLE_FREE_MULTI_WINDOW : WindowStyleType::WINDOW_STYLE_DEFAULT);
            windowManagementMode_ = WindowManagementMode::UNDEFINED;
        }
        return WMError::WM_OK;
    } else {
        return WMError::WM_ERROR_NULLPTR;
    }
}

WMError SessionManagerAgentController::UnregisterWindowManagerAgent(const sptr<IWindowManagerAgent>& windowManagerAgent,
    WindowManagerAgentType type, int32_t pid)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "type: %{public}u", static_cast<uint32_t>(type));
    if (smAgentContainer_.UnregisterAgent(windowManagerAgent, type)) {
        std::lock_guard<std::mutex> lock(windowManagerAgentPidMapMutex_);
        auto it = windowManagerPidAgentMap_.find(pid);
        if (it != windowManagerPidAgentMap_.end()) {
            auto& typeAgentMap = it->second;
            auto typeAgentIter = typeAgentMap.find(type);
            if (typeAgentIter != typeAgentMap.end()) {
                windowManagerAgentPairMap_.erase((typeAgentIter->second)->AsObject());
                typeAgentMap.erase(type);
                if (typeAgentMap.empty()) {
                    TLOGI(WmsLogTag::WMS_MAIN, "erase pid: %{public}d, type: %{public}u",
                        pid, static_cast<uint32_t>(type));
                    windowManagerPidAgentMap_.erase(pid);
                }
            }
        }
        return WMError::WM_OK;
    } else {
        return WMError::WM_ERROR_NULLPTR;
    }
}

void SessionManagerAgentController::UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing)
{
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_FLOAT)) {
        if (agent != nullptr) {
            agent->UpdateCameraFloatWindowStatus(accessTokenId, isShowing);
        }
    }
}

void SessionManagerAgentController::UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool isFocused)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "in");
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS)) {
        if (agent != nullptr) {
            agent->UpdateFocusChangeInfo(focusChangeInfo, isFocused);
        } else {
            TLOGW(WmsLogTag::WMS_FOCUS, "agent is invalid");
        }
    }
}

void SessionManagerAgentController::UpdateDisplayGroupInfo(DisplayGroupId displayGroupId, DisplayId displayId,
                                                           bool isAdd)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "in");
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_DISPLAYGROUP_INFO)) {
        if (agent != nullptr) {
            TLOGD(WmsLogTag::WMS_FOCUS, "Get agent for display group notification success");
            agent->UpdateDisplayGroupInfo(displayGroupId, displayId, isAdd);
        } else {
            TLOGW(WmsLogTag::WMS_FOCUS, "agent is invalid");
        }
    }
}

void SessionManagerAgentController::UpdateWindowModeTypeInfo(WindowModeType type)
{
    TLOGD(WmsLogTag::WMS_MAIN, "SessionManagerAgentController UpdateWindowModeTypeInfo type: %{public}d",
        static_cast<uint8_t>(type));
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_MODE)) {
        if (agent != nullptr) {
            agent->UpdateWindowModeTypeInfo(type);
        }
    }
}

void SessionManagerAgentController::NotifyAccessibilityWindowInfo(
    const std::vector<sptr<AccessibilityWindowInfo>>& infos, WindowUpdateType type)
{
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE)) {
        if (agent != nullptr) {
            agent->NotifyAccessibilityWindowInfo(infos, type);
        }
    }
}

void SessionManagerAgentController::NotifyWaterMarkFlagChangedResult(bool hasWaterMark)
{
    WLOGFD("NotifyWaterMarkFlagChanged with result:%{public}u", static_cast<uint32_t>(hasWaterMark));
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WATER_MARK_FLAG)) {
        if (agent != nullptr) {
            agent->NotifyWaterMarkFlagChangedResult(hasWaterMark);
        }
    }
}

void SessionManagerAgentController::UpdateWindowVisibilityInfo(
    const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos)
{
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY)) {
        agent->UpdateWindowVisibilityInfo(windowVisibilityInfos);
    }
}

void SessionManagerAgentController::UpdateVisibleWindowNum(
    const std::vector<VisibleWindowNumInfo>& visibleWindowNumInfo)
{
    for (const auto& num : visibleWindowNumInfo) {
        TLOGI(WmsLogTag::WMS_MAIN, "displayId=%{public}d, visibleWindowNum=%{public}d",
            num.displayId, num.visibleWindowNum);
    }
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_VISIBLE_WINDOW_NUM)) {
        if (agent != nullptr) {
            agent->UpdateVisibleWindowNum(visibleWindowNumInfo);
        }
    }
}

void SessionManagerAgentController::UpdateWindowDrawingContentInfo(
    const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos)
{
    WLOGFD("Size:%{public}zu", windowDrawingContentInfos.size());
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE)) {
        agent->UpdateWindowDrawingContentInfo(windowDrawingContentInfos);
    }
}

void SessionManagerAgentController::UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "accessTokenId:%{private}u, isShowing:%{public}d", accessTokenId,
        static_cast<int>(isShowing));
    for (auto &agent: smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_WINDOW)) {
        if (agent != nullptr) {
            agent->UpdateCameraWindowStatus(accessTokenId, isShowing);
        }
    }
}

void SessionManagerAgentController::DoAfterAgentDeath(const sptr<IRemoteObject>& remoteObject)
{
    std::lock_guard<std::mutex> lock(windowManagerAgentPidMapMutex_);
    auto it = windowManagerAgentPairMap_.find(remoteObject);
    if (it != windowManagerAgentPairMap_.end()) {
        auto [pid, type] = it->second;
        auto pidIter = windowManagerPidAgentMap_.find(pid);
        if (pidIter != windowManagerPidAgentMap_.end()) {
            auto& typeAgentMap = pidIter->second;
            TLOGI(WmsLogTag::WMS_MAIN, "type: %{public}u", static_cast<uint32_t>(type));
            typeAgentMap.erase(type);
            if (typeAgentMap.empty()) {
                TLOGI(WmsLogTag::WMS_MAIN, "pid: %{public}d", pid);
                windowManagerPidAgentMap_.erase(pid);
            }
        }
        windowManagerAgentPairMap_.erase(remoteObject);
    }
}

void SessionManagerAgentController::NotifyGestureNavigationEnabledResult(bool enable)
{
    WLOGFD("result:%{public}d", enable);
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GESTURE_NAVIGATION_ENABLED)) {
        if (agent != nullptr) {
            agent->NotifyGestureNavigationEnabledResult(enable);
        }
    }
}

void SessionManagerAgentController::NotifyWindowStyleChange(WindowStyleType type)
{
    TLOGD(WmsLogTag::WMS_MAIN, "windowStyletype: %{public}d",
          static_cast<uint8_t>(type));
    bool hasNotified = false;
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_STYLE)) {
        if (agent != nullptr) {
            agent->NotifyWindowStyleChange(type);
            hasNotified = true;
        }
    }
    if (!hasNotified) {
        windowManagementMode_ = type == WindowStyleType::WINDOW_STYLE_FREE_MULTI_WINDOW ?
            WindowManagementMode::FREEFORM : WindowManagementMode::FULLSCREEN;
    }
}

void SessionManagerAgentController::NotifyCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "notify persistentId: %{public}d, pid: %{public}d, "
        "displayId: %{public}d, userId: %{public}d", callingWindowInfo.windowId_,
        callingWindowInfo.callingPid_, static_cast<int32_t>(callingWindowInfo.displayId_), callingWindowInfo.userId_);
    for (const auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CALLING_DISPLAY)) {
        if (agent != nullptr) {
            agent->NotifyCallingWindowDisplayChanged(callingWindowInfo);
        } else {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "agent is nullptr, userId: %{public}d, displayId: %{public}d, "
                "persistentId: %{public}d", callingWindowInfo.userId_,
                static_cast<int32_t>(callingWindowInfo.displayId_), callingWindowInfo.windowId_);
        }
    }
}

void SessionManagerAgentController::NotifyWindowPidVisibilityChanged(
    const sptr<WindowPidVisibilityInfo>& info)
{
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_PID_VISIBILITY)) {
        if (agent != nullptr) {
            agent->NotifyWindowPidVisibilityChanged(info);
        }
    }
}

void SessionManagerAgentController::UpdatePiPWindowStateChanged(const std::string& bundleName, bool isForeground)
{
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PIP)) {
        if (agent != nullptr) {
            agent->UpdatePiPWindowStateChanged(bundleName, isForeground);
        }
    }
}

void SessionManagerAgentController::NotifyWindowSystemBarPropertyChange(
    WindowType type, const SystemBarProperty& systemBarProperty)
{
    for (auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_STATUS_BAR_PROPERTY)) {
        if (agent != nullptr) {
            agent->NotifyWindowSystemBarPropertyChange(type, systemBarProperty);
        }
    }
}

void SessionManagerAgentController::NotifyWindowPropertyChange(
    uint32_t propertyDirtyFlags, const WindowInfoList& windowInfoList)
{
    for (const auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY)) {
        if (agent != nullptr) {
            agent->NotifyWindowPropertyChange(propertyDirtyFlags, windowInfoList);
        }
    }
}

void SessionManagerAgentController::NotifySupportRotationChange(const SupportRotationInfo& supportRotationInfo)
{
    for (const auto& agent : smAgentContainer_.GetAgentsByType(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_SUPPORT_ROTATION)) {
        if (agent != nullptr) {
            agent->NotifySupportRotationChange(supportRotationInfo);
        } else {
            TLOGE(WmsLogTag::WMS_ROTATION, "agent is nullptr");
        }
    }
}
} // namespace Rosen
} // namespace OHOS