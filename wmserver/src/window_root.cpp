/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "window_root.h"
#include <ability_manager_client.h>
#include <cinttypes>
#include <hisysevent.h>
#include <hitrace_meter.h>
#include <transaction/rs_transaction.h>

#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
#include <display_power_mgr_client.h>
#endif

#include "display_manager_service_inner.h"
#include "permission.h"
#include "rs_adapter.h"
#include "window_helper.h"
#include "window_inner_manager.h"
#include "window_manager_hilog.h"
#include "window_manager_service.h"
#include "window_manager_service_utils.h"
#include "window_manager_agent_controller.h"
#include "window_system_effect.h"
#ifdef MEMMGR_WINDOW_ENABLE
#include "mem_mgr_client.h"
#include "mem_mgr_window_info.h"
#endif
namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "Root"};
int Comp(const std::pair<uint64_t, WindowVisibilityState>& a, const std::pair<uint64_t, WindowVisibilityState>& b)
{
    return a.first < b.first;
}
}

uint32_t WindowRoot::GetTotalWindowNum() const
{
    return static_cast<uint32_t>(windowNodeMap_.size());
}

sptr<WindowNode> WindowRoot::GetWindowForDumpAceHelpInfo() const
{
    for (auto& iter : windowNodeMap_) {
        if (iter.second->GetWindowType() == WindowType::WINDOW_TYPE_DESKTOP ||
            iter.second->GetWindowType() == WindowType::WINDOW_TYPE_NAVIGATION_BAR ||
            iter.second->GetWindowType() == WindowType::WINDOW_TYPE_STATUS_BAR ||
            iter.second->GetWindowType() == WindowType::WINDOW_TYPE_KEYGUARD) {
            return iter.second;
        }
    }
    return nullptr;
}

ScreenId WindowRoot::GetScreenGroupId(DisplayId displayId, bool& isRecordedDisplay)
{
    for (auto iter : displayIdMap_) {
        auto displayIdVec = iter.second;
        if (std::find(displayIdVec.begin(), displayIdVec.end(), displayId) != displayIdVec.end()) {
            isRecordedDisplay = true;
            return iter.first;
        }
    }
    isRecordedDisplay = false;
    WLOGFE("Current display is not be recorded, displayId: %{public}" PRIu64 "", displayId);
    return DisplayManagerServiceInner::GetInstance().GetScreenGroupIdByDisplayId(displayId);
}

sptr<WindowNodeContainer> WindowRoot::GetOrCreateWindowNodeContainer(DisplayId displayId)
{
    auto container = GetWindowNodeContainer(displayId);
    if (container != nullptr) {
        return container;
    }

    // In case of have no container for default display, create container
    WLOGI("Create container for current display, displayId: %{public}" PRIu64 "", displayId);
    sptr<DisplayInfo> displayInfo = DisplayManagerServiceInner::GetInstance().GetDisplayById(displayId);
    DisplayId defaultDisplayId = DisplayManagerServiceInner::GetInstance().GetDefaultDisplayId();
    return CreateWindowNodeContainer(defaultDisplayId, displayInfo);
}

sptr<WindowNodeContainer> WindowRoot::GetWindowNodeContainer(DisplayId displayId)
{
    bool isRecordedDisplay;
    ScreenId displayGroupId = GetScreenGroupId(displayId, isRecordedDisplay);
    auto iter = windowNodeContainerMap_.find(displayGroupId);
    if (iter != windowNodeContainerMap_.end()) {
        // if container exist for screenGroup and display is not be recorded, process expand display
        if (!isRecordedDisplay) {
            sptr<DisplayInfo> displayInfo = DisplayManagerServiceInner::GetInstance().GetDisplayById(displayId);
            // add displayId in displayId vector
            displayIdMap_[displayGroupId].push_back(displayId);
            auto displayRectMap = GetAllDisplayRectsByDMS(displayInfo);
            DisplayId defaultDisplayId = DisplayManagerServiceInner::GetInstance().GetDefaultDisplayId();
            ProcessExpandDisplayCreate(defaultDisplayId, displayInfo, displayRectMap);
        }
        return iter->second;
    }
    return nullptr;
}

sptr<WindowNodeContainer> WindowRoot::CreateWindowNodeContainer(DisplayId defaultDisplayId,
    sptr<DisplayInfo> displayInfo)
{
    if (displayInfo == nullptr || !CheckDisplayInfo(displayInfo)) {
        WLOGFE("get display failed or get invalid display info");
        return nullptr;
    }

    DisplayGroupInfo::GetInstance().SetDefaultDisplayId(defaultDisplayId);
    DisplayId displayId = displayInfo->GetDisplayId();
    ScreenId displayGroupId = displayInfo->GetScreenGroupId();
    WLOGI("create new container for display, width: %{public}d, height: %{public}d, "
        "displayGroupId:%{public}" PRIu64", displayId:%{public}" PRIu64"", displayInfo->GetWidth(),
        displayInfo->GetHeight(), displayGroupId, displayId);
    sptr<WindowNodeContainer> container = new WindowNodeContainer(displayInfo, displayGroupId);
    windowNodeContainerMap_.insert(std::make_pair(displayGroupId, container));
    std::vector<DisplayId> displayVec = { displayId };
    displayIdMap_.insert(std::make_pair(displayGroupId, displayVec));
    if (container == nullptr) {
        WLOGFE("create container failed, displayId :%{public}" PRIu64 "", displayId);
        return nullptr;
    }
    container->GetDisplayGroupController()->SetSplitRatioConfig(splitRatioConfig_);
    return container;
}

bool WindowRoot::CheckDisplayInfo(const sptr<DisplayInfo>& display)
{
    const int32_t minWidth = 50;
    const int32_t minHeight = 50;
    const int32_t maxWidth = 7680;
    const int32_t maxHeight = 7680; // 8k resolution
    if (display->GetWidth() < minWidth || display->GetWidth() > maxWidth ||
        display->GetHeight() < minHeight || display->GetHeight() > maxHeight) {
        return false;
    }
    return true;
}

sptr<WindowNode> WindowRoot::GetWindowNode(uint32_t windowId) const
{
    auto iter = windowNodeMap_.find(windowId);
    if (iter == windowNodeMap_.end()) {
        return nullptr;
    }
    return iter->second;
}

sptr<WindowNode> WindowRoot::GetWindowNodeByMissionId(uint32_t missionId) const
{
    using ValueType = const std::map<uint32_t, sptr<WindowNode>>::value_type&;
    auto it = std::find_if(windowNodeMap_.begin(), windowNodeMap_.end(), [missionId] (ValueType item) {
        return item.second && item.second->abilityInfo_.missionId_ == static_cast<int32_t>(missionId);
    });
    return it == windowNodeMap_.end() ? nullptr : it->second;
}

void WindowRoot::GetBackgroundNodesByScreenId(ScreenId screenGroupId, std::vector<sptr<WindowNode>>& windowNodes)
{
    for (const auto& it : windowNodeMap_) {
        if (it.second == nullptr) {
            continue;
        }
        wptr<WindowNodeContainer> container = GetWindowNodeContainer(it.second->GetDisplayId());
        if (container == nullptr) {
            continue;
        }
        auto iter = std::find_if(windowNodeContainerMap_.begin(), windowNodeContainerMap_.end(),
            [container](const std::map<uint64_t, sptr<WindowNodeContainer>>::value_type& containerPair) {
                return container.promote() == containerPair.second;
            });
        ScreenId screenGroupIdOfNode = INVALID_SCREEN_ID;
        if (iter != windowNodeContainerMap_.end()) {
            screenGroupIdOfNode = iter->first;
        }
        if (screenGroupId == screenGroupIdOfNode && !it.second->currentVisibility_) {
            windowNodes.push_back(it.second);
        }
    }
}

void WindowRoot::GetForegroundNodes(std::vector<sptr<WindowNode>>& windowNodes)
{
    for (const auto& it : windowNodeMap_) {
        if (it.second == nullptr) {
            continue;
        }
        if (it.second->currentVisibility_) {
            windowNodes.push_back(it.second);
        }
    }
}

sptr<WindowNode> WindowRoot::FindWindowNodeWithToken(const sptr<IRemoteObject>& token) const
{
    if (token == nullptr) {
        WLOGFE("token is null");
        return nullptr;
    }
    auto iter = std::find_if(windowNodeMap_.begin(), windowNodeMap_.end(),
        [token](const std::map<uint32_t, sptr<WindowNode>>::value_type& pair) {
            if ((WindowHelper::IsMainWindow(pair.second->GetWindowType())) ||
                (pair.second->GetWindowType() == WindowType::WINDOW_TYPE_DESKTOP)) {
                return pair.second->abilityToken_ == token;
            }
            return false;
        });
    if (iter == windowNodeMap_.end()) {
        WLOGE("cannot find windowNode");
        return nullptr;
    }
    return iter->second;
}

void WindowRoot::AddDeathRecipient(sptr<WindowNode> node)
{
    if (node == nullptr || node->GetWindowToken() == nullptr) {
        WLOGFE("failed, node is nullptr");
        return;
    }
    WLOGFD("Add for window: %{public}u", node->GetWindowId());

    auto remoteObject = node->GetWindowToken()->AsObject();
    windowIdMap_.insert(std::make_pair(remoteObject, node->GetWindowId()));

    if (windowDeath_ == nullptr) {
        WLOGE("failed to create death Recipient ptr WindowDeathRecipient");
        return;
    }
    if (!remoteObject->AddDeathRecipient(windowDeath_)) {
        WLOGE("failed to add death recipient");
    }
}

WMError WindowRoot::SaveWindow(const sptr<WindowNode>& node)
{
    if (node == nullptr) {
        WLOGFE("add window failed, node is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }

    WLOGFD("save windowId %{public}u", node->GetWindowId());
    windowNodeMap_.insert(std::make_pair(node->GetWindowId(), node));
    if (node->surfaceNode_ != nullptr) {
        surfaceIdWindowNodeMap_.insert(std::make_pair(node->surfaceNode_->GetId(), node));
        if (WindowHelper::IsMainWindow(node->GetWindowType())) {
            // Register FirstFrame Callback to rs, inform ability to get snapshot
            wptr<WindowNode> weak = node;
            auto firstFrameCompleteCallback = [weak]() {
                auto weakNode = weak.promote();
                if (weakNode == nullptr) {
                    WLOGFE("windowNode is nullptr");
                    return;
                }
                WindowInnerManager::GetInstance().CompleteFirstFrameDrawing(weakNode);
            };
            node->surfaceNode_->SetBufferAvailableCallback(firstFrameCompleteCallback);
        }
    }
    AddDeathRecipient(node);
    if (WindowHelper::IsMainWindow(node->GetWindowType())) {
        WindowInfoReporter::GetInstance().InsertCreateReportInfo(node->abilityInfo_.bundleName_);
    }
    return WMError::WM_OK;
}

WMError WindowRoot::MinimizeStructuredAppWindowsExceptSelf(sptr<WindowNode>& node)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "root:MinimizeStructuredAppWindowsExceptSelf");
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("MinimizeAbility failed, window container could not be found");
        return WMError::WM_ERROR_NULLPTR;
    }
    return container->MinimizeStructuredAppWindowsExceptSelf(node);
}

void WindowRoot::MinimizeTargetWindows(std::vector<uint32_t>& windowIds)
{
    for (auto& windowId : windowIds) {
        if (windowNodeMap_.count(windowId) != 0) {
            auto windowNode = windowNodeMap_[windowId];
            if (windowNode->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
                MinimizeApp::AddNeedMinimizeApp(windowNode, MinimizeReason::GESTURE_ANIMATION);
            } else {
                WLOGFE("Minimize window failed id: %{public}u, type: %{public}u",
                    windowNode->GetWindowId(), static_cast<uint32_t>(windowNode->GetWindowType()));
            }
        } else {
            WLOGFW("Cannot find window with id: %{public}u", windowId);
        }
    }
}

std::vector<sptr<WindowNode>> WindowRoot::GetSplitScreenWindowNodes(DisplayId displayId)
{
    auto container = GetOrCreateWindowNodeContainer(displayId);
    if (container == nullptr) {
        return {};
    }
    auto displayGroupController = container->GetDisplayGroupController();
    if (displayGroupController == nullptr) {
        return {};
    }
    auto windowPair = displayGroupController->GetWindowPairByDisplayId(displayId);
    if (windowPair == nullptr) {
        return {};
    }
    return windowPair->GetPairedWindows();
}

bool WindowRoot::IsForbidDockSliceMove(DisplayId displayId) const
{
    auto container = const_cast<WindowRoot*>(this)->GetOrCreateWindowNodeContainer(displayId);
    if (container == nullptr) {
        WLOGFE("can't find container");
        return true;
    }
    return container->IsForbidDockSliceMove(displayId);
}

bool WindowRoot::IsDockSliceInExitSplitModeArea(DisplayId displayId) const
{
    auto container = const_cast<WindowRoot*>(this)->GetOrCreateWindowNodeContainer(displayId);
    if (container == nullptr) {
        WLOGFE("can't find container");
        return false;
    }
    return container->IsDockSliceInExitSplitModeArea(displayId);
}

void WindowRoot::ExitSplitMode(DisplayId displayId)
{
    auto container = GetOrCreateWindowNodeContainer(displayId);
    if (container == nullptr) {
        WLOGFE("can't find container");
        return;
    }
    container->ExitSplitMode(displayId);
}

void WindowRoot::AddSurfaceNodeIdWindowNodePair(uint64_t surfaceNodeId, sptr<WindowNode> node)
{
    surfaceIdWindowNodeMap_.insert(std::make_pair(surfaceNodeId, node));
}

static void FillUnreliableWindowInfo(const sptr<WindowNode>& windowNode,
    std::vector<sptr<UnreliableWindowInfo>>& infos)
{
    if (windowNode == nullptr) {
        WLOGFW("null window node.");
        return;
    }
    sptr<UnreliableWindowInfo> info = new (std::nothrow) UnreliableWindowInfo();
    if (info == nullptr) {
        WLOGFE("null info.");
        return;
    }
    info->windowId_ = static_cast<int32_t>(windowNode->GetWindowId());
    info->windowRect_ = windowNode->GetWindowRect();
    info->zOrder_ = windowNode->zOrder_;
    infos.emplace_back(info);
    WLOGFI("windowId = %{public}d", info->windowId_);
}

static bool CheckUnreliableWindowType(WindowType windowType)
{
    if (windowType == WindowType::WINDOW_TYPE_APP_SUB_WINDOW ||
        windowType == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT ||
        windowType == WindowType::WINDOW_TYPE_TOAST) {
        return true;
    }
    WLOGFI("false, WindowType = %{public}d", windowType);
    return false;
}

void WindowRoot::GetUnreliableWindowInfo(int32_t windowId, std::vector<sptr<UnreliableWindowInfo>>& infos) const
{
    WLOGFD("Called.");
    for (const auto& [winId, windowNode] : windowNodeMap_) {
        if (windowNode == nullptr) {
            WLOGFW("null window node");
            continue;
        }
        int32_t curWindowId = static_cast<int32_t>(winId);
        if (curWindowId == windowId) {
            WLOGFI("windowId: %{public}d is parameter chosen", curWindowId);
            FillUnreliableWindowInfo(windowNode, infos);
            continue;
        }
        if (windowNode->GetWindowProperty()->GetTouchable()) {
            TLOGND(WmsLogTag::WMS_ATTRIBUTE, "PersistentId: %{public}d is touchable", curWindowId);
            continue;
        }
        if (!windowNode->currentVisibility_) {
            WLOGFD("windowId: %{public}d is not visible", curWindowId);
            continue;
        }
        WLOGFD("name = %{public}s, windowId = %{public}d, winType = %{public}d, "
            "visible = %{public}d", windowNode->GetWindowName().c_str(),
            curWindowId, windowNode->GetWindowType(), windowNode->currentVisibility_);
        if (CheckUnreliableWindowType(windowNode->GetWindowType())) {
            WLOGFI("windowId = %{public}d, WindowType = %{public}d", curWindowId, windowNode->GetWindowType());
            FillUnreliableWindowInfo(windowNode, infos);
        }
    }
}

void WindowRoot::GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) const
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("Get Visible Window Permission Denied");
    }
    for (auto [surfaceId, _] : lastVisibleData_) {
        auto iter = surfaceIdWindowNodeMap_.find(surfaceId);
        if (iter == surfaceIdWindowNodeMap_.end()) {
            continue;
        }
        sptr<WindowNode> node = iter->second;
        if (node == nullptr) {
            continue;
        }
        infos.emplace_back(new WindowVisibilityInfo(node->GetWindowId(), node->GetCallingPid(),
            node->GetCallingUid(), node->GetVisibilityState(), node->GetWindowType()));
    }
}

std::vector<std::pair<uint64_t, WindowVisibilityState>> WindowRoot::GetWindowVisibilityChangeInfo(
    std::shared_ptr<RSOcclusionData> occlusionData)
{
    std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilityChangeInfo;
    VisibleData& rsVisibleData = occlusionData->GetVisibleData();
    std::vector<std::pair<uint64_t, WindowVisibilityState> > currVisibleData;
    for (auto iter = rsVisibleData.begin(); iter != rsVisibleData.end(); iter++) {
        if (static_cast<WindowLayerState>(iter->second) < WINDOW_LAYER_DRAWING) {
            currVisibleData.emplace_back(iter->first, static_cast<WindowVisibilityState>(iter->second));
        }
    }
    std::sort(currVisibleData.begin(), currVisibleData.end(), Comp);
    uint32_t i, j;
    i = j = 0;
    for (; i < lastVisibleData_.size() && j < currVisibleData.size();) {
        if (lastVisibleData_[i].first < currVisibleData[j].first) {
            visibilityChangeInfo.emplace_back(lastVisibleData_[i].first, WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
            i++;
        } else if (lastVisibleData_[i].first > currVisibleData[j].first) {
            visibilityChangeInfo.emplace_back(currVisibleData[j].first, currVisibleData[j].second);
            j++;
        } else {
            if (lastVisibleData_[i].second != currVisibleData[j].second) {
                visibilityChangeInfo.emplace_back(currVisibleData[j].first, currVisibleData[j].second);
            }
            i++;
            j++;
        }
    }
    for (; i < lastVisibleData_.size(); ++i) {
        visibilityChangeInfo.emplace_back(lastVisibleData_[i].first, WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    }
    for (; j < currVisibleData.size(); ++j) {
        visibilityChangeInfo.emplace_back(currVisibleData[j].first, currVisibleData[j].second);
    }
    lastVisibleData_ = currVisibleData;
    return visibilityChangeInfo;
}

void WindowRoot::NotifyWindowVisibilityChange(std::shared_ptr<RSOcclusionData> occlusionData)
{
    std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilityChangeInfo =
        GetWindowVisibilityChangeInfo(occlusionData);
    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
#ifdef MEMMGR_WINDOW_ENABLE
    std::vector<sptr<Memory::MemMgrWindowInfo>> memMgrWindowInfos;
#endif
    for (const auto& elem : visibilityChangeInfo) {
        uint64_t surfaceId = elem.first;
        WindowVisibilityState visibilityState = elem.second;
        bool isVisible = visibilityState < WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION;
        auto iter = surfaceIdWindowNodeMap_.find(surfaceId);
        if (iter == surfaceIdWindowNodeMap_.end()) {
            continue;
        }
        sptr<WindowNode> node = iter->second;
        if (node == nullptr) {
            continue;
        }
        node->SetVisibilityState(visibilityState);
        windowVisibilityInfos.emplace_back(new WindowVisibilityInfo(node->GetWindowId(), node->GetCallingPid(),
            node->GetCallingUid(), visibilityState, node->GetWindowType()));
#ifdef MEMMGR_WINDOW_ENABLE
        memMgrWindowInfos.emplace_back(new Memory::MemMgrWindowInfo(node->GetWindowId(), node->GetCallingPid(),
            node->GetCallingUid(), isVisible));
#endif
        WLOGFD("NotifyWindowVisibilityChange: covered status changed window:%{public}u, visibilityState:%{public}d",
            node->GetWindowId(), visibilityState);
    }
    CheckAndNotifyWaterMarkChangedResult();
    if (windowVisibilityInfos.size() != 0) {
        WLOGI("Notify windowvisibilityinfo changed start");
        WindowManagerAgentController::GetInstance().UpdateWindowVisibilityInfo(windowVisibilityInfos);
    }
#ifdef MEMMGR_WINDOW_ENABLE
    if (memMgrWindowInfos.size() != 0) {
        WLOGI("Notify memMgrWindowInfos changed start");
        Memory::MemMgrClient::GetInstance().OnWindowVisibilityChanged(memMgrWindowInfos);
    }
#endif
}

AvoidArea WindowRoot::GetAvoidAreaByType(uint32_t windowId, AvoidAreaType avoidAreaType)
{
    AvoidArea avoidArea;
    sptr<WindowNode> node = GetWindowNode(windowId);
    if (node == nullptr) {
        WLOGFE("could not find window");
        return avoidArea;
    }
    sptr<WindowNodeContainer> container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("add window failed, window container could not be found");
        return avoidArea;
    }
    return container->GetAvoidAreaByType(node, avoidAreaType);
}

void WindowRoot::MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId)
{
    auto container = GetOrCreateWindowNodeContainer(displayId);
    if (container == nullptr) {
        WLOGFE("can't find window node container, failed!");
        return;
    }
    return container->MinimizeAllAppWindows(displayId, excludeWindowId);
}

WMError WindowRoot::ToggleShownStateForAllAppWindows()
{
    std::vector<DisplayId> displays = DisplayGroupInfo::GetInstance().GetAllDisplayIds();
    std::vector<sptr<WindowNodeContainer>> containers;
    bool isAllAppWindowsEmpty = true;
    for (auto displayId : displays) {
        auto container = GetOrCreateWindowNodeContainer(displayId);
        if (container == nullptr) {
            WLOGFE("can't find window node container, failed!");
            continue;
        }
        containers.emplace_back(container);
        isAllAppWindowsEmpty = isAllAppWindowsEmpty && container->IsAppWindowsEmpty();
    }
    WMError res = WMError::WM_OK;
    std::for_each(containers.begin(), containers.end(),
        [this, isAllAppWindowsEmpty, &res] (sptr<WindowNodeContainer> container) {
        auto restoreFunc = [this](uint32_t windowId, WindowMode mode) {
            auto windowNode = GetWindowNode(windowId);
            if (windowNode == nullptr) {
                return false;
            }
            if (!windowNode->GetWindowToken()) {
                return false;
            }
            auto property = windowNode->GetWindowToken()->GetWindowProperty();
            if (property == nullptr) {
                return false;
            }
            if (mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
                mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) {
                property->SetWindowMode(mode);
                // when change mode, need to reset shadow and radius
                windowNode->SetWindowMode(mode);
                WindowSystemEffect::SetWindowEffect(windowNode);
                windowNode->GetWindowToken()->RestoreSplitWindowMode(static_cast<uint32_t>(mode));
            }
            windowNode->GetWindowToken()->UpdateWindowState(WindowState::STATE_SHOWN);
            WindowManagerService::GetInstance().AddWindow(property);
            return true;
        };
        WMError tmpRes = container->ToggleShownStateForAllAppWindows(restoreFunc, isAllAppWindowsEmpty);
        res = (res == WMError::WM_OK) ? tmpRes : res;
    });
    return res;
}

void WindowRoot::NotifyMMIServiceOnline()
{
    TLOGI(WmsLogTag::WMS_EVENT, "called");
    for (const auto& [windowId, window] : windowNodeMap_) {
        if (window == nullptr) {
            WLOGFE("window is nullptr.");
            return;
        }
        if (window->GetWindowToken()) {
            TLOGI(WmsLogTag::WMS_EVENT, "Id:%{public}u", windowId);
            window->GetWindowToken()->NotifyMMIServiceOnline(windowId);
        }
    }
}

void WindowRoot::DestroyLeakStartingWindow()
{
    TLOGD(WmsLogTag::WMS_STARTUP_PAGE, "called");
    std::vector<uint32_t> destroyIds;
    for (auto& iter : windowNodeMap_) {
        if (iter.second->startingWindowShown_ && !iter.second->GetWindowToken()) {
            destroyIds.push_back(iter.second->GetWindowId());
        }
    }
    for (auto id : destroyIds) {
        TLOGD(WmsLogTag::WMS_STARTUP_PAGE, "Id:%{public}u", id);
        DestroyWindow(id, false);
    }
}

WMError WindowRoot::PostProcessAddWindowNode(sptr<WindowNode>& node, sptr<WindowNode>& parentNode,
    sptr<WindowNodeContainer>& container)
{
    if (!node->currentVisibility_) {
        WLOGW("window is invisible, do not need process");
        return WMError::WM_DO_NOTHING;
    }
    if (WindowHelper::IsSubWindow(node->GetWindowType())) {
        if (parentNode == nullptr) {
            WLOGFE("window type is invalid");
            return WMError::WM_ERROR_INVALID_TYPE;
        }
        sptr<WindowNode> parent = nullptr;
        container->RaiseZOrderForAppWindow(parentNode, parent);
    }
    if (node->GetWindowProperty()->GetFocusable()) {
        // when launcher reboot, the focus window should not change with showing a full screen window.
        sptr<WindowNode> focusWin = GetWindowNode(container->GetFocusWindow());
        if (focusWin == nullptr ||
            !(WindowHelper::IsFullScreenWindow(focusWin->GetWindowMode()) && focusWin->zOrder_ > node->zOrder_)) {
            WLOGFI("set focus window on id:%{public}d", node->GetWindowId());
            container->SetFocusWindow(node->GetWindowId());
            container->DumpScreenWindowTree();
            needCheckFocusWindow = true;
        }
    }
    if (!WindowHelper::IsSystemBarWindow(node->GetWindowType())) {
        container->SetActiveWindow(node->GetWindowId(), false);
    }

    for (auto& child : node->children_) {
        if (child == nullptr || !child->currentVisibility_) {
            break;
        }
        HandleKeepScreenOn(child->GetWindowId(), child->IsKeepScreenOn());
    }
    HandleKeepScreenOn(node->GetWindowId(), node->IsKeepScreenOn());
    WLOGFD("windowId:%{public}u, name:%{public}s, orientation:%{public}u, type:%{public}u, isMainWindow:%{public}d",
        node->GetWindowId(), node->GetWindowName().c_str(), static_cast<uint32_t>(node->GetRequestedOrientation()),
        node->GetWindowType(), WindowHelper::IsMainWindow(node->GetWindowType()));
    if (WindowHelper::IsRotatableWindow(node->GetWindowType(), node->GetWindowMode())) {
        if (node->stateMachine_.IsShowAnimationPlaying()) {
            WLOGFD("[FixOrientation] window is playing show animation, do not update display orientation");
            return WMError::WM_OK;
        }
        auto topRotatableWindow = container->GetNextRotatableWindow(INVALID_WINDOW_ID);
        if (topRotatableWindow == node) {
            container->SetDisplayOrientationFromWindow(node, true);
        }
    }

    if (node->GetWindowType() == WindowType::WINDOW_TYPE_LAUNCHER_RECENT) {
        std::vector<sptr<WindowNode>> windowNodes;
        container->TraverseContainer(windowNodes);
        for (auto& winNode : windowNodes) {
            if (winNode && WindowHelper::IsMainWindow(winNode->GetWindowType()) &&
                winNode->GetVisibilityState() < WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION &&
                winNode->GetWindowToken()) {
                winNode->GetWindowToken()->NotifyForegroundInteractiveStatus(false);
            }
        }
    }

    return WMError::WM_OK;
}

bool WindowRoot::CheckAddingModeAndSize(sptr<WindowNode>& node, const sptr<WindowNodeContainer>& container)
{
    if (!WindowHelper::IsMainWindow(node->GetWindowType())) {
        return true;
    }
    // intercept the node which doesn't support floating mode at tile mode
    if (WindowHelper::IsInvalidWindowInTileLayoutMode(node->GetWindowModeSupportType(),
        container->GetCurrentLayoutMode())) {
        WLOGFE("window doesn't support floating mode in tile, windowId: %{public}u", node->GetWindowId());
        return false;
    }
    // intercept the node that the tile rect can't be applied to
    WMError res = container->IsTileRectSatisfiedWithSizeLimits(node);
    if (res != WMError::WM_OK) {
        return false;
    }
    return true;
}

Rect WindowRoot::GetDisplayRectWithoutSystemBarAreas(const sptr<WindowNode> dstNode)
{
    DisplayId displayId = dstNode->GetDisplayId();
    std::map<WindowType, std::pair<bool, Rect>> systemBarRects;
    for (const auto& it : windowNodeMap_) {
        auto& node = it.second;
        if (node && (node->GetDisplayId() == displayId) &&
            WindowHelper::IsSystemBarWindow(node->GetWindowType())) {
            systemBarRects[node->GetWindowType()] = std::make_pair(node->currentVisibility_, node->GetWindowRect());
        }
    }
    auto container = GetOrCreateWindowNodeContainer(displayId);
    if (container == nullptr) {
        WLOGFE("failed, window container could not be found");
        return {0, 0, 0, 0}; // empty rect
    }
    auto displayRect = DisplayGroupInfo::GetInstance().GetDisplayRect(displayId);
    Rect targetRect = displayRect;
    auto displayInfo = DisplayGroupInfo::GetInstance().GetDisplayInfo(displayId);
    if (displayInfo && WmsUtils::IsExpectedRotatableWindow(dstNode->GetRequestedOrientation(),
        displayInfo->GetDisplayOrientation(), dstNode->GetWindowMode(), dstNode->GetWindowFlags())) {
        WLOGFD("[FixOrientation] the window is expected rotatable, pre-calculated");
        targetRect.height_ = displayRect.width_;
        targetRect.width_ = displayRect.height_;
        return targetRect;
    }

    bool isStatusShow = true;
    if (systemBarRects.count(WindowType::WINDOW_TYPE_STATUS_BAR)) {
        isStatusShow = systemBarRects[WindowType::WINDOW_TYPE_STATUS_BAR].first;
        targetRect.posY_ = displayRect.posY_ + static_cast<int32_t>(
            systemBarRects[WindowType::WINDOW_TYPE_STATUS_BAR].second.height_);
        targetRect.height_ -= systemBarRects[WindowType::WINDOW_TYPE_STATUS_BAR].second.height_;
        WLOGFD("after status bar winRect:[x:%{public}d, y:%{public}d, w:%{public}d, h:%{public}d]",
            targetRect.posX_, targetRect.posY_, targetRect.width_, targetRect.height_);
    }
    if (systemBarRects.count(WindowType::WINDOW_TYPE_NAVIGATION_BAR)) {
        if (isStatusShow && !(systemBarRects[WindowType::WINDOW_TYPE_NAVIGATION_BAR].first)) {
            return targetRect;
        }
        targetRect.height_ -= systemBarRects[WindowType::WINDOW_TYPE_NAVIGATION_BAR].second.height_;
        WLOGFD("after navi bar winRect:[x:%{public}d, y:%{public}d, w:%{public}d, h:%{public}d]",
            targetRect.posX_, targetRect.posY_, targetRect.width_, targetRect.height_);
    }
    return targetRect;
}

void WindowRoot::GetAllAnimationPlayingNodes(std::vector<wptr<WindowNode>>& windowNodes)
{
    for (const auto& it : windowNodeMap_) {
        if (it.second) {
            if (!WindowHelper::IsMainWindow(it.second->GetWindowType())) {
                continue;
            }
            WLOGFD("id:%{public}u state:%{public}u",
                it.second->GetWindowId(), static_cast<uint32_t>(it.second->stateMachine_.GetCurrentState()));
            if (it.second->stateMachine_.IsRemoteAnimationPlaying() ||
                it.second->stateMachine_.GetAnimationCount() > 0) {
                windowNodes.emplace_back(it.second);
            }
        }
    }
}

void WindowRoot::LayoutWhenAddWindowNode(sptr<WindowNode>& node, bool afterAnimation)
{
    if (node == nullptr) {
        WLOGFE("failed, node is nullptr");
        return;
    }
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("add window failed, window container could not be found");
        return;
    }

    if (!CheckAddingModeAndSize(node, container)) { // true means stop adding
        WLOGFE("Invalid mode or size in tile mode, windowId: %{public}u", node->GetWindowId());
        return;
    }

    container->LayoutWhenAddWindowNode(node, afterAnimation);
    return;
}

WMError WindowRoot::BindDialogToParent(sptr<WindowNode>& node, sptr<WindowNode>& parentNode)
{
    if (node->GetWindowType() != WindowType::WINDOW_TYPE_DIALOG) {
        return WMError::WM_OK;
    }
    sptr<WindowNode> callerNode = FindMainWindowWithToken(node->dialogTargetToken_);
    parentNode = (callerNode != nullptr) ? callerNode : nullptr;
    if (parentNode == nullptr) {
        node->GetWindowToken()->NotifyDestroy();
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    return WMError::WM_OK;
}

WMError WindowRoot::AddWindowNode(uint32_t parentId, sptr<WindowNode>& node, bool fromStartingWin)
{
    if (node == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }

    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        return WMError::WM_ERROR_INVALID_DISPLAY;
    }

    if (!CheckAddingModeAndSize(node, container)) { // true means stop adding
        /*
         * Starting Window has no windowToken, which should be destroied if mode or size is invalid
         */
        if (node->GetWindowToken() == nullptr) {
            (void)DestroyWindow(node->GetWindowId(), false);
        }
        WLOGFE("Invalid mode or size in tile mode, windowId: %{public}u", node->GetWindowId());
        return WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE;
    }

    if (fromStartingWin) {
        if (WindowHelper::IsFullScreenWindow(node->GetWindowMode()) &&
            WindowHelper::IsAppWindow(node->GetWindowType()) && !node->isPlayAnimationShow_) {
            WMError res = MinimizeStructuredAppWindowsExceptSelf(node);
            if (res != WMError::WM_OK) {
                WLOGFE("Minimize other structured window failed");
                MinimizeApp::ClearNodesWithReason(MinimizeReason::OTHER_WINDOW);
                return res;
            }
        }
        WMError res = container->ShowStartingWindow(node);
        if (res != WMError::WM_OK) {
            MinimizeApp::ClearNodesWithReason(MinimizeReason::OTHER_WINDOW);
        }
        return res;
    }
    if (WindowHelper::IsAppFullOrSplitWindow(node->GetWindowType(), node->GetWindowMode())) {
        container->NotifyDockWindowStateChanged(node, false);
    }
    // limit number of main window
    uint32_t mainWindowNumber = container->GetWindowCountByType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    if (mainWindowNumber >= maxAppWindowNumber_ && node->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        container->MinimizeOldestAppWindow();
    }

    auto parentNode = GetWindowNode(parentId);

    WMError res = BindDialogToParent(node, parentNode);
    if (res != WMError::WM_OK) {
        return res;
    }

    res = container->AddWindowNode(node, parentNode);
    if (res != WMError::WM_OK) {
        WLOGFE("failed with ret: %{public}u", static_cast<uint32_t>(res));
        return res;
    }
    return PostProcessAddWindowNode(node, parentNode, container);
}

WMError WindowRoot::RemoveWindowNode(uint32_t windowId, bool fromAnimation)
{
    WLOGFD("begin");
    auto node = GetWindowNode(windowId);
    if (node == nullptr) {
        WLOGFE("could not find window");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("failed, window container could not be found");
        return WMError::WM_ERROR_INVALID_DISPLAY;
    }
    container->DropShowWhenLockedWindowIfNeeded(node);
    UpdateFocusWindowWithWindowRemoved(node, container);
    UpdateActiveWindowWithWindowRemoved(node, container);
    UpdateBrightnessWithWindowRemoved(windowId, container);
    WMError res = container->RemoveWindowNode(node, fromAnimation);
    if (res == WMError::WM_OK) {
        for (auto& child : node->children_) {
            if (child == nullptr) {
                break;
            }
            HandleKeepScreenOn(child->GetWindowId(), false);
        }
        HandleKeepScreenOn(windowId, false);
    }

    if (node->GetWindowType() == WindowType::WINDOW_TYPE_LAUNCHER_RECENT) {
        std::vector<sptr<WindowNode>> windowNodes;
        container->TraverseContainer(windowNodes);
        for (auto& winNode : windowNodes) {
            if (winNode && WindowHelper::IsMainWindow(winNode->GetWindowType()) &&
                winNode->GetVisibilityState() < WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION &&
                winNode->GetWindowToken()) {
                winNode->GetWindowToken()->NotifyForegroundInteractiveStatus(true);
            }
        }
    }

    return res;
}

void WindowRoot::UpdateDisplayOrientationWhenHideWindow(sptr<WindowNode>& node)
{
    if (!FIX_ORIENTATION_ENABLE) {
        return;
    }
    WLOGFD("[FixOrientation] begin");
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("[FixOrientation] failed, window container could not be found");
        return;
    }
    auto nextRotatableWindow = container->GetNextRotatableWindow(node->GetWindowId());
    if (nextRotatableWindow != nullptr) {
        WLOGFD("[FixOrientation] next rotatable window: %{public}u", nextRotatableWindow->GetWindowId());
        container->SetDisplayOrientationFromWindow(nextRotatableWindow, false);
    }
}

WMError WindowRoot::SetGestureNavigationEnabled(bool enable)
{
    if (lastGestureNativeEnabled_ == enable) {
        WLOGFW("Do not set gesture navigation too much times as same value and the value is %{public}d", enable);
        return WMError::WM_DO_NOTHING;
    }
    WindowManagerAgentController::GetInstance().NotifyGestureNavigationEnabledResult(enable);
    lastGestureNativeEnabled_ = enable;
    WLOGFD("Set gesture navigation enabled succeeded and notify result of %{public}d", enable);
    return WMError::WM_OK;
}

WMError WindowRoot::UpdateWindowNode(uint32_t windowId, WindowUpdateReason reason)
{
    auto node = GetWindowNode(windowId);
    if (node == nullptr) {
        WLOGFE("could not find window");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("update window failed, window container could not be found");
        return WMError::WM_ERROR_INVALID_DISPLAY;
    }

    auto ret = container->UpdateWindowNode(node, reason);
    if (ret == WMError::WM_OK && reason == WindowUpdateReason::UPDATE_FLAGS) {
        CheckAndNotifyWaterMarkChangedResult();
    }
    return ret;
}

WMError WindowRoot::UpdateSizeChangeReason(uint32_t windowId, WindowSizeChangeReason reason)
{
    auto node = GetWindowNode(windowId);
    if (node == nullptr) {
        WLOGFE("could not find window");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("failed, window container could not be found");
        return WMError::WM_ERROR_INVALID_DISPLAY;
    }
    container->UpdateSizeChangeReason(node, reason);
    return WMError::WM_OK;
}

void WindowRoot::SetBrightness(uint32_t windowId, float brightness)
{
    auto node = GetWindowNode(windowId);
    if (node == nullptr) {
        WLOGFE("could not find window");
        return;
    }
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("failed, window container could not be found");
        return;
    }
    if (!WindowHelper::IsAppWindow(node->GetWindowType())) {
        WLOGW("Only app window support set brightness");
        return;
    }
    if (windowId != container->GetActiveWindow()) {
        WLOGE("Window is not active with windowId:%{public}d", windowId);
        return;
    }
    if (std::fabs(brightness - UNDEFINED_BRIGHTNESS) <= std::numeric_limits<float>::min()) {
        if (std::fabs(container->GetDisplayBrightness() - brightness) > std::numeric_limits<float>::min()) {
            WLOGFI("value: %{public}f to restore brightness", brightness);
#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
            DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().RestoreBrightness();
#endif
            container->SetDisplayBrightness(brightness);
        }
    } else {
        if (std::fabs(container->GetDisplayBrightness() - brightness) > std::numeric_limits<float>::min()) {
            WLOGFI("value: %{public}u", container->ToOverrideBrightness(brightness));
#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
            DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().OverrideBrightness(
                container->ToOverrideBrightness(brightness));
#endif
            container->SetDisplayBrightness(brightness);
        }
    }
    container->SetBrightnessWindow(windowId);
}

void WindowRoot::HandleKeepScreenOn(uint32_t windowId, bool requireLock)
{
    auto node = GetWindowNode(windowId);
    if (node == nullptr) {
        WLOGFE("could not find window");
        return;
    }
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("failed, window container could not be found");
        return;
    }
    container->HandleKeepScreenOn(node, requireLock);
}

void WindowRoot::UpdateFocusableProperty(uint32_t windowId)
{
    auto node = GetWindowNode(windowId);
    if (node == nullptr) {
        WLOGFE("could not find window");
        return;
    }
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("failed, window container could not be found");
        return;
    }

    if (windowId != container->GetFocusWindow() || node->GetWindowProperty()->GetFocusable()) {
        return;
    }
    auto nextFocusableWindow = container->GetNextFocusableWindow(windowId);
    if (nextFocusableWindow != nullptr) {
        WLOGI("Next focus window id: %{public}u", nextFocusableWindow->GetWindowId());
        container->SetFocusWindow(nextFocusableWindow->GetWindowId());
    }
}

WMError WindowRoot::SetWindowMode(sptr<WindowNode>& node, WindowMode dstMode)
{
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("failed, window container could not be found");
        return WMError::WM_ERROR_INVALID_DISPLAY;
    }
    WindowMode curWinMode = node->GetWindowMode();
    if (curWinMode == dstMode) {
        return WMError::WM_OK;
    }
    auto res = container->SetWindowMode(node, dstMode);
    auto nextRotatableWindow = container->GetNextRotatableWindow(0);
    if (nextRotatableWindow != nullptr) {
        DisplayManagerServiceInner::GetInstance().SetOrientationFromWindow(nextRotatableWindow->GetDisplayId(),
            nextRotatableWindow->GetRequestedOrientation());
    }
    return res;
}

WMError WindowRoot::DestroyWindowSelf(sptr<WindowNode>& node, const sptr<WindowNodeContainer>& container)
{
    for (auto& child : node->children_) {
        if (child == nullptr) {
            continue;
        }
        child->parent_ = nullptr;
        if ((child->GetWindowToken() != nullptr) && (child->abilityToken_ != node->abilityToken_) &&
            (child->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG)) {
            child->GetWindowToken()->NotifyDestroy();
        }
    }
    std::vector<uint32_t> windowIds;
    WMError res = container->DestroyWindowNode(node, windowIds);
    if (res != WMError::WM_OK) {
        WLOGFE("RemoveWindowNode failed");
    }
    return DestroyWindowInner(node);
}

WMError WindowRoot::DestroyWindowWithChild(sptr<WindowNode>& node, const sptr<WindowNodeContainer>& container)
{
    auto token = node->abilityToken_;
    std::vector<uint32_t> windowIds;
    WMError res = container->DestroyWindowNode(node, windowIds);
    for (auto id : windowIds) {
        node = GetWindowNode(id);
        if (!node) {
            continue;
        }
        HandleKeepScreenOn(id, false);
        DestroyWindowInner(node);
        if ((node->GetWindowToken() != nullptr) && (node->abilityToken_ != token) &&
            (node->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG)) {
            node->GetWindowToken()->NotifyDestroy();
        }
    }
    return res;
}

WMError WindowRoot::DestroyWindow(uint32_t windowId, bool onlySelf)
{
    auto node = GetWindowNode(windowId);
    if (node == nullptr) {
        WLOGFE("failed, because window node is not exist.");
        return WMError::WM_ERROR_NULLPTR;
    }
    WLOGI("windowId %{public}u, onlySelf:%{public}u.", windowId, onlySelf);
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (!container) {
        WLOGFW("failed, window container could not be found");
        return DestroyWindowInner(node);
    }

    UpdateFocusWindowWithWindowRemoved(node, container);
    UpdateActiveWindowWithWindowRemoved(node, container);
    UpdateBrightnessWithWindowRemoved(windowId, container);
    HandleKeepScreenOn(windowId, false);
    if (onlySelf) {
        return DestroyWindowSelf(node, container);
    } else {
        return DestroyWindowWithChild(node, container);
    }
}

WMError WindowRoot::DestroyWindowInner(sptr<WindowNode>& node)
{
    if (node == nullptr) {
        WLOGFE("window has been destroyed");
        return WMError::WM_ERROR_DESTROYED_OBJECT;
    }

    if (node->GetVisibilityState() < WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
        std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
        node->SetVisibilityState(WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
        windowVisibilityInfos.emplace_back(new WindowVisibilityInfo(node->GetWindowId(), node->GetCallingPid(),
            node->GetCallingUid(), node->GetVisibilityState(), node->GetWindowType()));
        WLOGFD("NotifyWindowVisibilityChange: covered status changed window:%{public}u, visibilityState:%{public}d",
            node->GetWindowId(), node->GetVisibilityState());
        WindowManagerAgentController::GetInstance().UpdateWindowVisibilityInfo(windowVisibilityInfos);

        CheckAndNotifyWaterMarkChangedResult();
    }

    auto cmpFunc = [node](const std::map<uint64_t, sptr<WindowNode>>::value_type& pair) {
        if (pair.second == nullptr) {
            return false;
        }
        if (pair.second->GetWindowId() == node->GetWindowId()) {
            return true;
        }
        return false;
    };
    auto iter = std::find_if(surfaceIdWindowNodeMap_.begin(), surfaceIdWindowNodeMap_.end(), cmpFunc);
    if (iter != surfaceIdWindowNodeMap_.end()) {
        surfaceIdWindowNodeMap_.erase(iter);
    }

    sptr<IWindow> window = node->GetWindowToken();
    if ((window != nullptr) && (window->AsObject() != nullptr)) {
        if (windowIdMap_.count(window->AsObject()) == 0) {
            WLOGE("window remote object has been destroyed");
            return WMError::WM_ERROR_DESTROYED_OBJECT;
        }

        if (window->AsObject() != nullptr) {
            window->AsObject()->RemoveDeathRecipient(windowDeath_);
        }
        windowIdMap_.erase(window->AsObject());
    }
    windowNodeMap_.erase(node->GetWindowId());
    WLOGI("destroy window use_count:%{public}d", node->GetSptrRefCount());
    return WMError::WM_OK;
}

void WindowRoot::UpdateFocusWindowWithWindowRemoved(const sptr<WindowNode>& node,
    const sptr<WindowNodeContainer>& container) const
{
    if (node == nullptr || container == nullptr) {
        WLOGFE("window is invalid");
        return;
    }
    if (node->GetWindowType() == WindowType::WINDOW_TYPE_DOCK_SLICE) {
        WLOGI("window is divider, do not get next focus window.");
        return;
    }
    uint32_t windowId = node->GetWindowId();
    uint32_t focusedWindowId = container->GetFocusWindow();
    WLOGFI("current window: %{public}u, focus window: %{public}u", windowId, focusedWindowId);
    container->DumpScreenWindowTree();
    if (windowId != focusedWindowId) {
        auto iter = std::find_if(node->children_.begin(), node->children_.end(),
            [focusedWindowId](sptr<WindowNode> node) {
                return node->GetWindowId() == focusedWindowId;
            });
        if (iter == node->children_.end()) {
            return;
        }
    }
    if (!node->children_.empty()) {
        auto firstChild = node->children_.front();
        if (firstChild->priority_ < 0) {
            windowId = firstChild->GetWindowId();
        }
    }

    auto nextFocusableWindow = container->GetNextFocusableWindow(windowId);
    if (nextFocusableWindow != nullptr) {
        WLOGFI("adjust focus window, next focus window id: %{public}u", nextFocusableWindow->GetWindowId());
        container->SetFocusWindow(nextFocusableWindow->GetWindowId());
    } else {
        WLOGFW("next focus window is invalid");
        container->SetFocusWindow(INVALID_WINDOW_ID);
    }
}

void WindowRoot::UpdateActiveWindowWithWindowRemoved(const sptr<WindowNode>& node,
    const sptr<WindowNodeContainer>& container) const
{
    if (node == nullptr || container == nullptr) {
        WLOGFE("window is invalid");
        return;
    }
    uint32_t windowId = node->GetWindowId();
    uint32_t activeWindowId = container->GetActiveWindow();
    WLOGFD("current window: %{public}u, active window: %{public}u", windowId, activeWindowId);
    if (windowId != activeWindowId) {
        auto iter = std::find_if(node->children_.begin(), node->children_.end(),
            [activeWindowId](sptr<WindowNode> node) {
                return node->GetWindowId() == activeWindowId;
            });
        if (iter == node->children_.end()) {
            return;
        }
    }
    if (!node->children_.empty()) {
        auto firstChild = node->children_.front();
        if (firstChild->priority_ < 0) {
            windowId = firstChild->GetWindowId();
        }
    }

    auto nextActiveWindow = container->GetNextActiveWindow(windowId);
    if (nextActiveWindow != nullptr) {
        WLOGI("Next active window id: %{public}u", nextActiveWindow->GetWindowId());
        container->SetActiveWindow(nextActiveWindow->GetWindowId(), true);
    }
}

void WindowRoot::UpdateBrightnessWithWindowRemoved(uint32_t windowId, const sptr<WindowNodeContainer>& container) const
{
    if (container == nullptr) {
        WLOGFE("window container could not be found");
        return;
    }
    if (windowId == container->GetBrightnessWindow()) {
        WLOGFD("winId: %{public}u", container->GetActiveWindow());
        container->UpdateBrightness(container->GetActiveWindow(), true);
    }
}

bool WindowRoot::IsVerticalDisplay(sptr<WindowNode>& node) const
{
    auto container = const_cast<WindowRoot*>(this)->GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("get display direction failed, window container could not be found");
        return false;
    }
    return container->IsVerticalDisplay(node->GetDisplayId());
}

WMError WindowRoot::RequestFocus(uint32_t windowId)
{
    auto node = GetWindowNode(windowId);
    if (node == nullptr) {
        WLOGFE("could not find window");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!node->currentVisibility_) {
        WLOGFE("could not request focus before it does not be shown");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("window container could not be found");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (node->GetWindowProperty()->GetFocusable()) {
        return container->SetFocusWindow(windowId);
    }
    return WMError::WM_ERROR_INVALID_OPERATION;
}

WMError WindowRoot::RequestActiveWindow(uint32_t windowId)
{
    auto node = GetWindowNode(windowId);
    if (node == nullptr) {
        WLOGFE("could not find window");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (WindowHelper::IsSystemBarWindow(node->GetWindowType())) {
        WLOGFE("window could not be active window");
        return WMError::WM_ERROR_INVALID_TYPE;
    }
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("window container could not be found");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto res = container->SetActiveWindow(windowId, false);
    WLOGFD("windowId:%{public}u, name:%{public}s, orientation:%{public}u, type:%{public}u, isMainWindow:%{public}d",
        windowId, node->GetWindowName().c_str(), static_cast<uint32_t>(node->GetRequestedOrientation()),
        node->GetWindowType(), WindowHelper::IsMainWindow(node->GetWindowType()));
    return res;
}

void WindowRoot::ProcessWindowStateChange(WindowState state, WindowStateChangeReason reason)
{
    for (auto& elem : windowNodeContainerMap_) {
        if (elem.second == nullptr) {
            continue;
        }
        elem.second->ProcessWindowStateChange(state, reason);
    }
}

void WindowRoot::NotifySystemBarTints()
{
    WLOGFD("notify current system bar tints");
    for (auto& it : windowNodeContainerMap_) {
        if (it.second != nullptr) {
            it.second->NotifySystemBarTints(displayIdMap_[it.first]);
        }
    }
}

WMError WindowRoot::NotifyDesktopUnfrozen()
{
    WLOGFD("notify desktop unfrozen");
    for (const auto& it : windowNodeMap_) {
        auto& node = it.second;
        // just need notify desktop unfrozen when desktop shown
        // since unfrozen will change window state to shown
        if (node && (node->GetWindowType() == WindowType::WINDOW_TYPE_DESKTOP)
            && (node->GetWindowToken()) && node->currentVisibility_) {
            node->GetWindowToken()->UpdateWindowState(WindowState::STATE_UNFROZEN);
            return WMError::WM_OK;
        }
    }
    WLOGFD("notify desktop unfrozen failed, maybe no window node or windowToken!");
    return WMError::WM_ERROR_INVALID_OPERATION;
}

sptr<WindowNode> WindowRoot::FindWallpaperWindow()
{
    auto iter = std::find_if(windowNodeMap_.begin(), windowNodeMap_.end(),
        [](const std::map<uint32_t, sptr<WindowNode>>::value_type& pair) {
            return pair.second->GetWindowType() == WindowType::WINDOW_TYPE_WALLPAPER;
        });
    if (iter == windowNodeMap_.end()) {
        WLOGI("cannot find windowNode");
        return nullptr;
    }
    return iter->second;
}

WMError WindowRoot::RaiseZOrderForAppWindow(sptr<WindowNode>& node)
{
    if (node == nullptr) {
        WLOGFW("add window failed, node is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (node->GetWindowType() == WindowType::WINDOW_TYPE_DOCK_SLICE) {
        auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
        if (container == nullptr) {
            WLOGFW("window container could not be found");
            return WMError::WM_ERROR_NULLPTR;
        }
        container->RaiseSplitRelatedWindowToTop(node);
        return WMError::WM_OK;
    }
    if (node->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) {
        auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
        if (container == nullptr) {
            WLOGFW("window container could not be found");
            return WMError::WM_ERROR_NULLPTR;
        }
        sptr<WindowNode> parentNode = FindMainWindowWithToken(node->dialogTargetToken_);
        if (parentNode != nullptr) {
            container->RaiseZOrderForAppWindow(node, parentNode);
        }
        return WMError::WM_OK;
    }

    if (!WindowHelper::IsAppWindow(node->GetWindowType())) {
        WLOGFW("window is not app window");
        return WMError::WM_ERROR_INVALID_TYPE;
    }
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFW("add window failed, window container could not be found");
        return WMError::WM_ERROR_NULLPTR;
    }

    auto parentNode = GetWindowNode(node->GetParentId());
    return container->RaiseZOrderForAppWindow(node, parentNode);
}

void WindowRoot::DispatchKeyEvent(sptr<WindowNode> node, std::shared_ptr<MMI::KeyEvent> event)
{
    sptr<WindowNodeContainer> container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFW("window container could not be found");
        return;
    }
    std::vector<sptr<WindowNode>> windowNodes;
    container->TraverseContainer(windowNodes);
    auto iter = std::find(windowNodes.begin(), windowNodes.end(), node);
    if (iter == windowNodes.end()) {
        WLOGFE("Cannot find node");
        return;
    }
    for (++iter; iter != windowNodes.end(); ++iter) {
        if (*iter == nullptr) {
            WLOGFE("Node is null");
            continue;
        }
        if ((*iter)->GetWindowType() == WindowType::WINDOW_TYPE_APP_COMPONENT) {
            WLOGFI("Skip component window: %{public}u", (*iter)->GetWindowId());
            continue;
        }
        if (WindowHelper::IsAppWindow((*iter)->GetWindowType())) {
            WLOGFI("App window: %{public}u", (*iter)->GetWindowId());
            if ((*iter)->GetWindowToken()) {
                (*iter)->GetWindowToken()->ConsumeKeyEvent(event);
            }
            break;
        }
        WLOGFI("Unexpected window: %{public}u", (*iter)->GetWindowId());
        break;
    }
}

uint32_t WindowRoot::GetWindowIdByObject(const sptr<IRemoteObject>& remoteObject)
{
    auto iter = windowIdMap_.find(remoteObject);
    return iter == std::end(windowIdMap_) ? INVALID_WINDOW_ID : iter->second;
}

void WindowRoot::OnRemoteDied(const sptr<IRemoteObject>& remoteObject)
{
    callback_(Event::REMOTE_DIED, remoteObject);
}

WMError WindowRoot::GetTopWindowId(uint32_t mainWinId, uint32_t& topWinId)
{
    if (windowNodeMap_.find(mainWinId) == windowNodeMap_.end()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto node = windowNodeMap_[mainWinId];
    if (!node->currentVisibility_) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!node->children_.empty()) {
        auto iter = node->children_.rbegin();
        if (WindowHelper::IsSubWindow((*iter)->GetWindowType()) ||
            WindowHelper::IsSystemSubWindow((*iter)->GetWindowType())) {
            topWinId = (*iter)->GetWindowId();
            return WMError::WM_OK;
        }
    }
    topWinId = mainWinId;
    return WMError::WM_OK;
}

WMError WindowRoot::SetWindowLayoutMode(DisplayId displayId, WindowLayoutMode mode)
{
    auto container = GetOrCreateWindowNodeContainer(displayId);
    if (container == nullptr) {
        WLOGFE("window container could not be found");
        return WMError::WM_ERROR_NULLPTR;
    }
    WMError ret = container->SwitchLayoutPolicy(mode, displayId, true);
    if (ret != WMError::WM_OK) {
        WLOGFW("set window layout mode failed displayId: %{public}" PRIu64 ", ret: %{public}d", displayId, ret);
    }
    return ret;
}

std::vector<DisplayId> WindowRoot::GetAllDisplayIds() const
{
    std::vector<DisplayId> displayIds;
    for (auto& it : windowNodeContainerMap_) {
        if (!it.second) {
            return {};
        }
        std::vector<DisplayId>& displayIdVec = const_cast<WindowRoot*>(this)->displayIdMap_[it.first];
        for (auto displayId : displayIdVec) {
            displayIds.push_back(displayId);
        }
    }
    return displayIds;
}

std::string WindowRoot::GenAllWindowsLogInfo() const
{
    std::ostringstream os;
    WindowNodeOperationFunc func = [&os](sptr<WindowNode> node) {
        if (node == nullptr) {
            WLOGE("WindowNode is nullptr");
            return false;
        }
        os<<"window_name:"<<node->GetWindowName()<<",id:"<<node->GetWindowId()<<
           ",focusable:"<<node->GetWindowProperty()->GetFocusable()<<";";
        return false;
    };

    for (auto& elem : windowNodeContainerMap_) {
        if (elem.second == nullptr) {
            continue;
        }
        std::vector<DisplayId>& displayIdVec = const_cast<WindowRoot*>(this)->displayIdMap_[elem.first];
        for (const auto& displayId : displayIdVec) {
            os << "Display " << displayId << ":";
        }
        elem.second->TraverseWindowTree(func, true);
    }
    return os.str();
}

void WindowRoot::FocusFaultDetection() const
{
    if (!needCheckFocusWindow) {
        return;
    }
    bool needReport = true;
    uint32_t focusWinId = INVALID_WINDOW_ID;
    for (auto& elem : windowNodeContainerMap_) {
        if (elem.second == nullptr) {
            continue;
        }
        focusWinId = elem.second->GetFocusWindow();
        if (focusWinId != INVALID_WINDOW_ID) {
            needReport = false;
            sptr<WindowNode> windowNode = GetWindowNode(focusWinId);
            if (windowNode == nullptr || !windowNode->currentVisibility_) {
                needReport = true;
                WLOGFE("The focus windowNode is nullptr or is invisible, focusWinId: %{public}u", focusWinId);
                break;
            }
        }
    }
    if (needReport) {
        std::string windowLog(GenAllWindowsLogInfo());
        WLOGFE("The focus window is faulty, focusWinId:%{public}u, %{public}s", focusWinId, windowLog.c_str());
        int32_t ret = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
            "NO_FOCUS_WINDOW",
            OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            "PID", getpid(),
            "UID", getuid(),
            "PACKAGE_NAME", "foundation",
            "PROCESS_NAME", "foundation",
            "MSG", windowLog);
        if (ret != 0) {
            WLOGFE("Write HiSysEvent error, ret:%{public}d", ret);
        }
    }
}

void WindowRoot::ProcessExpandDisplayCreate(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    std::map<DisplayId, Rect>& displayRectMap)
{
    if (displayInfo == nullptr || !CheckDisplayInfo(displayInfo)) {
        WLOGFE("get display failed or get invalid display info");
        return;
    }
    DisplayGroupInfo::GetInstance().SetDefaultDisplayId(defaultDisplayId);
    DisplayId displayId = displayInfo->GetDisplayId();
    ScreenId displayGroupId = displayInfo->GetScreenGroupId();
    auto container = windowNodeContainerMap_[displayGroupId];
    if (container == nullptr) {
        WLOGFE("window node container is nullptr, displayId :%{public}" PRIu64 "", displayId);
        return;
    }

    container->GetDisplayGroupController()->ProcessDisplayCreate(defaultDisplayId, displayInfo, displayRectMap);
    container->GetDisplayGroupController()->SetSplitRatioConfig(splitRatioConfig_);
    WLOGI("Container exist, add new display, displayId: %{public}" PRIu64"", displayId);
}

std::map<DisplayId, Rect> WindowRoot::GetAllDisplayRectsByDMS(sptr<DisplayInfo> displayInfo)
{
    std::map<DisplayId, Rect> displayRectMap;

    if (displayInfo == nullptr) {
        return displayRectMap;
    }

    for (auto& displayId : displayIdMap_[displayInfo->GetScreenGroupId()]) {
        auto info = DisplayManagerServiceInner::GetInstance().GetDisplayById(displayId);
        Rect displayRect = { info->GetOffsetX(), info->GetOffsetY(), info->GetWidth(), info->GetHeight() };
        displayRectMap.insert(std::make_pair(displayId, displayRect));

        WLOGI("displayId: %{public}" PRIu64", displayRect: [ %{public}d, %{public}d, %{public}d, %{public}d]",
            displayId, displayRect.posX_, displayRect.posY_, displayRect.width_, displayRect.height_);
    }
    return displayRectMap;
}

std::map<DisplayId, Rect> WindowRoot::GetAllDisplayRectsByDisplayInfo(
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap)
{
    std::map<DisplayId, Rect> displayRectMap;

    for (const auto& iter : displayInfoMap) {
        auto id = iter.first;
        auto info = iter.second;
        Rect displayRect = { info->GetOffsetX(), info->GetOffsetY(), info->GetWidth(), info->GetHeight() };
        displayRectMap.insert(std::make_pair(id, displayRect));

        WLOGI("displayId: %{public}" PRIu64", displayRect: [ %{public}d, %{public}d, %{public}d, %{public}d]",
            id, displayRect.posX_, displayRect.posY_, displayRect.width_, displayRect.height_);
    }
    return displayRectMap;
}

void WindowRoot::ProcessDisplayCreate(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap)
{
    DisplayId displayId = (displayInfo == nullptr) ? DISPLAY_ID_INVALID : displayInfo->GetDisplayId();
    ScreenId displayGroupId = (displayInfo == nullptr) ? SCREEN_ID_INVALID : displayInfo->GetScreenGroupId();
    auto iter = windowNodeContainerMap_.find(displayGroupId);
    if (iter == windowNodeContainerMap_.end()) {
        CreateWindowNodeContainer(defaultDisplayId, displayInfo);
        WLOGI("Create new container for display, displayId: %{public}" PRIu64"", displayId);
    } else {
        auto& displayIdVec = displayIdMap_[displayGroupId];
        if (std::find(displayIdVec.begin(), displayIdVec.end(), displayId) != displayIdVec.end()) {
            WLOGI("Current display is already exist, displayId: %{public}" PRIu64"", displayId);
            return;
        }
        // add displayId in displayId vector
        displayIdMap_[displayGroupId].push_back(displayId);
        auto displayRectMap = GetAllDisplayRectsByDisplayInfo(displayInfoMap);
        ProcessExpandDisplayCreate(defaultDisplayId, displayInfo, displayRectMap);
    }
}

void WindowRoot::MoveNotShowingWindowToDefaultDisplay(DisplayId defaultDisplayId, DisplayId displayId)
{
    for (auto& elem : windowNodeMap_) {
        auto& windowNode = elem.second;
        if (windowNode->GetDisplayId() == displayId && !windowNode->currentVisibility_) {
            std::vector<DisplayId> newShowingDisplays = { defaultDisplayId };
            windowNode->SetShowingDisplays(newShowingDisplays);
            windowNode->isShowingOnMultiDisplays_ = false;
            if (windowNode->GetWindowToken()) {
                windowNode->GetWindowToken()->UpdateDisplayId(windowNode->GetDisplayId(), defaultDisplayId);
            }
            windowNode->SetDisplayId(defaultDisplayId);
        }
    }
}

void WindowRoot::ProcessDisplayDestroy(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap)
{
    DisplayId displayId = (displayInfo == nullptr) ? DISPLAY_ID_INVALID : displayInfo->GetDisplayId();
    ScreenId displayGroupId = (displayInfo == nullptr) ? SCREEN_ID_INVALID : displayInfo->GetScreenGroupId();
    auto& displayIdVec = displayIdMap_[displayGroupId];

    auto iter = windowNodeContainerMap_.find(displayGroupId);
    if (iter == windowNodeContainerMap_.end() ||
        std::find(displayIdVec.begin(), displayIdVec.end(), displayId) == displayIdVec.end() ||
        displayInfoMap.find(displayId) == displayInfoMap.end()) {
        WLOGFE("could not find display, destroy failed, displayId: %{public}" PRIu64"", displayId);
        return;
    }

    // erase displayId in displayIdMap
    auto displayIter = std::remove(displayIdVec.begin(), displayIdVec.end(), displayId);
    displayIdVec.erase(displayIter, displayIdVec.end());

    // container process display destroy
    auto container = iter->second;
    if (container == nullptr) {
        WLOGFE("window node container is nullptr, displayId :%{public}" PRIu64 "", displayId);
        return;
    }
    WLOGI("displayId: %{public}" PRIu64"", displayId);

    std::vector<uint32_t> needDestroyWindows;
    auto displayRectMap = GetAllDisplayRectsByDisplayInfo(displayInfoMap);
    // erase displayId in displayRectMap
    auto displayRectIter = displayRectMap.find(displayId);
    if (displayRectIter == displayRectMap.end()) {
        return;
    }
    displayRectMap.erase(displayRectIter);
    container->GetDisplayGroupController()->ProcessDisplayDestroy(
        defaultDisplayId, displayInfo, displayRectMap, needDestroyWindows);
    for (auto id : needDestroyWindows) {
        auto node = GetWindowNode(id);
        if (node != nullptr) {
            DestroyWindowInner(node);
        }
    }
    // move window which is not showing on destroyed display to default display
    MoveNotShowingWindowToDefaultDisplay(defaultDisplayId, displayId);
    WLOGI("[Display Destroy] displayId: %{public}" PRIu64" ", displayId);
}

void WindowRoot::ProcessDisplayChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type)
{
    if (displayInfo == nullptr) {
        WLOGFE("get display failed");
        return;
    }
    DisplayId displayId = displayInfo->GetDisplayId();
    ScreenId displayGroupId = displayInfo->GetScreenGroupId();
    auto& displayIdVec = displayIdMap_[displayGroupId];
    auto iter = windowNodeContainerMap_.find(displayGroupId);
    if (iter == windowNodeContainerMap_.end() || std::find(displayIdVec.begin(),
        displayIdVec.end(), displayId) == displayIdVec.end()) {
        WLOGFE("[Display Change] could not find display, change failed, displayId: %{public}" PRIu64"", displayId);
        return;
    }
    // container process display change
    auto container = iter->second;
    if (container == nullptr) {
        WLOGFE("window node container is nullptr, displayId :%{public}" PRIu64 "", displayId);
        return;
    }

    auto displayRectMap = GetAllDisplayRectsByDisplayInfo(displayInfoMap);
    container->GetDisplayGroupController()->ProcessDisplayChange(defaultDisplayId, displayInfo, displayRectMap, type);
}

Rect WindowRoot::GetDisplayGroupRect(DisplayId displayId) const
{
    Rect fullDisplayRect;
    auto container = const_cast<WindowRoot*>(this)->GetOrCreateWindowNodeContainer(displayId);
    if (container == nullptr) {
        WLOGFE("window container could not be found");
        return fullDisplayRect;
    }
    return container->GetDisplayGroupRect();
}

bool WindowRoot::HasPrivateWindow(DisplayId displayId)
{
    auto container = GetWindowNodeContainer(displayId);
    return container != nullptr ? container->HasPrivateWindow() : false;
}

bool WindowRoot::HasMainFullScreenWindowShown(DisplayId displayId)
{
    auto container = GetWindowNodeContainer(displayId);
    return container != nullptr ? container->HasMainFullScreenWindowShown() : false;
}

void WindowRoot::SetMaxAppWindowNumber(uint32_t windowNum)
{
    maxAppWindowNumber_ = windowNum;
}

void WindowRoot::SetSplitRatios(const std::vector<float>& splitRatioNumbers)
{
    auto& splitRatios = splitRatioConfig_.splitRatios;
    splitRatios.clear();
    splitRatios = splitRatioNumbers;
    for (auto iter = splitRatios.begin(); iter != splitRatios.end();) {
        if (*iter > 0 && *iter < 1) { // valid ratio range (0, 1)
            iter++;
        } else {
            iter = splitRatios.erase(iter);
        }
    }
    std::sort(splitRatios.begin(), splitRatios.end());
    auto iter = std::unique(splitRatios.begin(), splitRatios.end());
    splitRatios.erase(iter, splitRatios.end()); // remove duplicate ratios
}

void WindowRoot::SetExitSplitRatios(const std::vector<float>& exitSplitRatios)
{
    if (exitSplitRatios.size() != 2) { // 2 is size of vector.
        return;
    }
    if (exitSplitRatios[0] > 0 && exitSplitRatios[0] < DEFAULT_SPLIT_RATIO) {
        splitRatioConfig_.exitSplitStartRatio = exitSplitRatios[0];
    }
    if (exitSplitRatios[1] > DEFAULT_SPLIT_RATIO && exitSplitRatios[1] < 1) {
        splitRatioConfig_.exitSplitEndRatio = exitSplitRatios[1];
    }
}

WMError WindowRoot::GetModeChangeHotZones(DisplayId displayId,
    ModeChangeHotZones& hotZones, const ModeChangeHotZonesConfig& config)
{
    auto container = GetOrCreateWindowNodeContainer(displayId);
    if (container == nullptr) {
        WLOGFE("GetModeChangeHotZones failed, window container could not be found");
        return WMError::WM_ERROR_NULLPTR;
    }
    container->GetModeChangeHotZones(displayId, hotZones, config);
    return WMError::WM_OK;
}

void WindowRoot::RemoveSingleUserWindowNodes(int accountId)
{
    std::vector<DisplayId> displayIds = GetAllDisplayIds();
    for (auto id : displayIds) {
        sptr<WindowNodeContainer> container = GetOrCreateWindowNodeContainer(id);
        if (container == nullptr) {
            WLOGW("get container failed %{public}" PRIu64"", id);
            continue;
        }
        container->RemoveSingleUserWindowNodes(accountId);
    }
}

WMError WindowRoot::UpdateRsTree(uint32_t windowId, bool isAdd)
{
    sptr<WindowNode> node = GetWindowNode(windowId);
    if (node == nullptr) {
        WLOGFE("could not find window");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto container = GetOrCreateWindowNodeContainer(node->GetDisplayId());
    if (container == nullptr) {
        WLOGFE("window container could not be found");
        return WMError::WM_ERROR_NULLPTR;
    }
    for (auto& displayId : node->GetShowingDisplays()) {
        if (isAdd) {
            container->AddNodeOnRSTree(node, displayId, displayId, WindowUpdateType::WINDOW_UPDATE_ACTIVE);
        } else {
            container->RemoveNodeFromRSTree(node, displayId, displayId, WindowUpdateType::WINDOW_UPDATE_ACTIVE);
        }
    }
    RSTransactionAdapter::FlushImplicitTransaction(node->GetRSUIContext());
    return WMError::WM_OK;
}

sptr<WindowNode> WindowRoot::FindMainWindowWithToken(sptr<IRemoteObject> token)
{
    auto iter = std::find_if(windowNodeMap_.begin(), windowNodeMap_.end(),
        [token](const std::map<uint32_t, sptr<WindowNode>>::value_type& pair) {
            if (WindowHelper::IsMainWindow(pair.second->GetWindowType())) {
                return pair.second->abilityToken_ == token;
            }
            return false;
        });
    if (iter == windowNodeMap_.end()) {
        WLOGI("cannot find windowNode");
        return nullptr;
    }
    return iter->second;
}

bool WindowRoot::CheckMultiDialogWindows(WindowType type, sptr<IRemoteObject> token)
{
    if (type != WindowType::WINDOW_TYPE_DIALOG) {
        return false;
    }

    sptr<WindowNode> newCaller, oriCaller;

    newCaller = FindMainWindowWithToken(token);
    if (newCaller == nullptr) {
        return false;
    }

    for (auto& iter : windowNodeMap_) {
        if (iter.second->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) {
            oriCaller = FindMainWindowWithToken(iter.second->dialogTargetToken_);
            if (oriCaller == newCaller) {
                return true;
            }
        }
    }

    return false;
}

sptr<WindowNode> WindowRoot::GetWindowNodeByAbilityToken(const sptr<IRemoteObject>& abilityToken)
{
    for (const auto& iter : windowNodeMap_) {
        if (iter.second != nullptr && iter.second->abilityToken_ == abilityToken) {
            return iter.second;
        }
    }
    WLOGFE("could not find required abilityToken!");
    return nullptr;
}

bool WindowRoot::TakeWindowPairSnapshot(DisplayId displayId)
{
    auto container = GetWindowNodeContainer(displayId);
    return container == nullptr ? false : container->TakeWindowPairSnapshot(displayId);
}

void WindowRoot::ClearWindowPairSnapshot(DisplayId displayId)
{
    auto container = GetWindowNodeContainer(displayId);
    if (container == nullptr) {
        WLOGFE("clear window pair snapshot failed, because container in null");
        return;
    }
    return container->ClearWindowPairSnapshot(displayId);
}

void WindowRoot::CheckAndNotifyWaterMarkChangedResult()
{
    auto searchWaterMarkWindow = [](wptr<WindowNode> node) {
        return (node != nullptr && node->GetVisibilityState() < WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION &&
                (node->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK)));
    };
    bool currentWaterMarkState = false;
    for (auto& containerPair : windowNodeContainerMap_) {
        auto container = containerPair.second;
        if (container == nullptr) {
            continue;
        }
        std::vector<sptr<WindowNode>> allWindowNode;
        container->TraverseContainer(allWindowNode);
        auto itor = std::find_if(allWindowNode.begin(), allWindowNode.end(), searchWaterMarkWindow);
        if (itor != allWindowNode.end()) {
            currentWaterMarkState = true;
            break;
        }
    }
    if (lastWaterMarkShowStates_ != currentWaterMarkState) {
        WLOGFD("WaterMarkWindows has been changed. lastWaterMarkState : %{public}d, newState:%{public}d",
            lastWaterMarkShowStates_, currentWaterMarkState);
        WindowManagerAgentController::GetInstance().NotifyWaterMarkFlagChangedResult(currentWaterMarkState);
        lastWaterMarkShowStates_ = currentWaterMarkState;
    }
}
} // namespace Rosen
} // namespace OHOS
