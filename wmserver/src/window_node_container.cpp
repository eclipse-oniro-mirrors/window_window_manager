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

#include "window_node_container.h"

#include <ability_manager_client.h>
#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <ctime>
#include <hitrace_meter.h>
#include <limits>
#include <transaction/rs_interfaces.h>
#include <transaction/rs_transaction.h>
#include <transaction/rs_sync_transaction_controller.h>
#include <type_traits>

#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
#include <display_power_mgr_client.h>
#endif

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

#include "common_event_manager.h"
#include "dm_common.h"
#include "remote_animation.h"
#include "rs_adapter.h"
#include "starting_window.h"
#include "window_helper.h"
#include "window_inner_manager.h"
#include "window_layout_policy_cascade.h"
#include "window_layout_policy_tile.h"
#include "window_manager_agent_controller.h"
#include "window_manager_hilog.h"
#include "window_manager_service.h"
#include "window_manager_service_utils.h"
#include "window_system_effect.h"
#include "wm_common.h"
#include "wm_common_inner.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "Container"};
constexpr int WINDOW_NAME_MAX_LENGTH = 10;
constexpr uint32_t MAX_BRIGHTNESS = 255;
constexpr uint32_t SPLIT_WINDOWS_CNT = 2;
constexpr uint32_t EXIT_SPLIT_POINTS_NUMBER = 2;
constexpr int UID_TRANSFROM_DIVISOR = 200000;
constexpr int UID_MIN = 100;
}
AnimationConfig WindowNodeContainer::animationConfig_;
bool WindowNodeContainer::isFloatWindowAboveFullWindow_ = false;
uint32_t WindowNodeContainer::maxMainFloatingWindowNumber_ = 100;
bool WindowNodeContainer::isAnimateTransactionEnabled_ = false;
WindowUIType WindowNodeContainer::windowUIType_ = WindowUIType::INVALID_WINDOW;

WindowNodeContainer::WindowNodeContainer(const sptr<DisplayInfo>& displayInfo, ScreenId displayGroupId)
{
    DisplayId displayId = displayInfo->GetDisplayId();

    // create and displayGroupInfo and displayGroupController
    DisplayGroupInfo::GetInstance().Init(displayGroupId, displayInfo);
    displayGroupController_ = new DisplayGroupController(this);
    displayGroupController_->InitNewDisplay(displayId);

    // init layout policy
    layoutPolicies_[WindowLayoutMode::CASCADE] = new WindowLayoutPolicyCascade(
            displayGroupController_->displayGroupWindowTree_);
    layoutPolicies_[WindowLayoutMode::TILE] = new WindowLayoutPolicyTile(
        displayGroupController_->displayGroupWindowTree_);
    layoutPolicy_ = layoutPolicies_[WindowLayoutMode::CASCADE];
    layoutPolicy_->Launch();

    // set initial divider rect in windowPair
    Rect initialDivRect = layoutPolicies_[WindowLayoutMode::CASCADE]->GetDividerRect(displayId);
    auto windowPair = displayGroupController_->GetWindowPairByDisplayId(displayId);
    if (windowPair != nullptr) {
        windowPair->SetDividerRect(initialDivRect);
    }

    // init avoidAreaController
    avoidController_ = new AvoidAreaController(focusedWindow_);
    WindowInnerManager::GetInstance().NotifyDisplayLimitRectChange(
        DisplayGroupInfo::GetInstance().GetAllDisplayRects());
    isAnimateTransactionEnabled_ = system::GetParameter("persist.window.animateTransaction.enabled", "1")  == "1";
}

WindowNodeContainer::~WindowNodeContainer()
{
    Destroy();
}

uint32_t WindowNodeContainer::GetWindowCountByType(WindowType windowType)
{
    uint32_t windowNumber = 0;
    auto counter = [&windowNumber, &windowType](sptr<WindowNode>& windowNode) {
        if (windowNode->GetWindowType() == windowType && !windowNode->startingWindowShown_) {
            ++windowNumber;
        }
    };
    std::for_each(belowAppWindowNode_->children_.begin(), belowAppWindowNode_->children_.end(), counter);
    std::for_each(appWindowNode_->children_.begin(), appWindowNode_->children_.end(), counter);
    std::for_each(aboveAppWindowNode_->children_.begin(), aboveAppWindowNode_->children_.end(), counter);
    return windowNumber;
}

uint32_t WindowNodeContainer::GetMainFloatingWindowCount()
{
    uint32_t windowNumber = 0;
    auto counter = [&windowNumber](sptr<WindowNode>& windowNode) {
        WindowType windowType = windowNode->GetWindowType();
        WindowMode windowMode = windowNode->GetWindowMode();
        if (WindowHelper::IsMainFloatingWindow(windowType, windowMode) &&
            !windowNode->startingWindowShown_) {
            ++windowNumber;
        }
    };
    std::for_each(appWindowNode_->children_.begin(), appWindowNode_->children_.end(), counter);
    std::for_each(aboveAppWindowNode_->children_.begin(), aboveAppWindowNode_->children_.end(), counter);
    return windowNumber;
}

WMError WindowNodeContainer::AddWindowNodeOnWindowTree(sptr<WindowNode>& node, const sptr<WindowNode>& parentNode)
{
    sptr<WindowNode> root = FindRoot(node->GetWindowType());
    if (root == nullptr && !(WindowHelper::IsSystemSubWindow(node->GetWindowType()) &&
        parentNode != nullptr)) {
        WLOGFE("root is nullptr!");
        return WMError::WM_ERROR_NULLPTR;
    }
    node->requestedVisibility_ = true;
    if (parentNode != nullptr) { // subwindow
        if (WindowHelper::IsSystemSubWindow(node->GetWindowType()) ||
            node->GetWindowType() == WindowType::WINDOW_TYPE_APP_COMPONENT) {
            if (WindowHelper::IsSubWindow(parentNode->GetWindowType()) ||
                WindowHelper::IsSystemSubWindow(parentNode->GetWindowType()) ||
                parentNode->GetWindowType() == WindowType::WINDOW_TYPE_APP_COMPONENT ||
                parentNode->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) {
                // some times, dialog is a child window, so exclude
                WLOGFE("the parent of window cannot be any sub window");
                return WMError::WM_ERROR_INVALID_PARAM;
            }
        } else {
            if (parentNode->parent_ != root &&
                !((parentNode->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED)) &&
                (parentNode->parent_ == aboveAppWindowNode_))) {
                WLOGFE("window type and parent window not match \
                    or try to add subwindow to subwindow, which is forbidden");
                return WMError::WM_ERROR_INVALID_PARAM;
            }
        }
        node->currentVisibility_ = parentNode->currentVisibility_;
        node->parent_ = parentNode;
    } else { // mainwindow
        node->parent_ = root;
        node->currentVisibility_ = true;
        for (auto& child : node->children_) {
            child->currentVisibility_ = child->requestedVisibility_;
        }
        if (WindowHelper::IsSystemBarWindow(node->GetWindowType())) {
            displayGroupController_->sysBarNodeMaps_[node->GetDisplayId()][node->GetWindowType()] = node;
        }
    }
    return WMError::WM_OK;
}

WMError WindowNodeContainer::ShowStartingWindow(sptr<WindowNode>& node)
{
    if (node->currentVisibility_) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "current window is visible, windowId: %{public}u",
            node->GetWindowId());
        return WMError::WM_ERROR_INVALID_OPERATION;
    }

    WMError res = AddWindowNodeOnWindowTree(node, nullptr);
    if (res != WMError::WM_OK) {
        return res;
    }
    UpdateWindowTree(node);
    displayGroupController_->PreProcessWindowNode(node, WindowUpdateType::WINDOW_UPDATE_ADDED);
    StartingWindow::AddNodeOnRSTree(node, layoutPolicy_->IsMultiDisplay());
    AssignZOrder();
    layoutPolicy_->PerformWindowLayout(node, WindowUpdateType::WINDOW_UPDATE_ADDED);
    TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "windowId: %{public}u end", node->GetWindowId());
    return WMError::WM_OK;
}

WMError WindowNodeContainer::IsTileRectSatisfiedWithSizeLimits(sptr<WindowNode>& node)
{
    if (layoutMode_ == WindowLayoutMode::TILE &&
        !layoutPolicy_->IsTileRectSatisfiedWithSizeLimits(node)) {
        WLOGFE("layoutMode is tile, default rect is not satisfied with size limits of window, windowId: %{public}u",
            node->GetWindowId());
        return WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE;
    }
    return WMError::WM_OK;
}

AnimationConfig& WindowNodeContainer::GetAnimationConfigRef()
{
    return animationConfig_;
}

void WindowNodeContainer::LayoutWhenAddWindowNode(sptr<WindowNode>& node, bool afterAnimation)
{
    if (afterAnimation) {
        layoutPolicy_->PerformWindowLayout(node, WindowUpdateType::WINDOW_UPDATE_ADDED);
        return;
    }
    WLOGFD("AddWindowNode Id:%{public}u, currState:%{public}u",
        node->GetWindowId(), static_cast<uint32_t>(node->stateMachine_.GetCurrentState()));
    if (WindowHelper::IsMainWindow(node->GetWindowType()) &&
        RemoteAnimation::IsRemoteAnimationEnabledAndFirst(node->GetDisplayId()) &&
        node->stateMachine_.IsShowAnimationPlaying()) {
        // for first frame callback
        auto winRect = node->GetWindowRect();
        if (node->surfaceNode_) {
            node->surfaceNode_->SetBounds(0, 0, winRect.width_, winRect.height_);
            WLOGI("SetBounds id:%{public}u, rect:[%{public}d, %{public}d, %{public}u, %{public}u]",
                node->GetWindowId(), winRect.posX_, winRect.posY_, winRect.width_, winRect.height_);
            layoutPolicy_->NotifyClientAndAnimation(node, winRect, WindowSizeChangeReason::UNDEFINED);
        }
    } else {
        if (node->GetWindowProperty()->GetAnimationFlag() == static_cast<uint32_t>(WindowAnimation::CUSTOM) &&
            WindowHelper::IsSystemWindow(node->GetWindowType())) {
                node->SetWindowSizeChangeReason(WindowSizeChangeReason::CUSTOM_ANIMATION_SHOW);
        }
        layoutPolicy_->PerformWindowLayout(node, WindowUpdateType::WINDOW_UPDATE_ADDED);
    }
}

WMError WindowNodeContainer::AddWindowNode(sptr<WindowNode>& node, sptr<WindowNode>& parentNode, bool afterAnimation)
{
    if (!node->startingWindowShown_) { // window except main Window
        WMError res = AddWindowNodeOnWindowTree(node, parentNode);
        if (res != WMError::WM_OK) {
            return res;
        }
        UpdateWindowTree(node);
        displayGroupController_->PreProcessWindowNode(node, WindowUpdateType::WINDOW_UPDATE_ADDED);
        // add node on RSTree
        for (auto& displayId : node->GetShowingDisplays()) {
            AddNodeOnRSTree(node, displayId, displayId, WindowUpdateType::WINDOW_UPDATE_ADDED,
                node->isPlayAnimationShow_);
        }
    } else { // only main app window has starting window
        node->isPlayAnimationShow_ = false;
        node->startingWindowShown_ = false;
        AddAppSurfaceNodeOnRSTree(node);
        ReZOrderShowWhenLockedWindowIfNeeded(node);
        RaiseZOrderForAppWindow(node, parentNode);
    }
    auto windowPair = displayGroupController_->GetWindowPairByDisplayId(node->GetDisplayId());
    if (windowPair == nullptr) {
        WLOGFE("Window pair is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    windowPair->UpdateIfSplitRelated(node);
    if (node->IsSplitMode()) {
        // raise the z-order of window pair
        RaiseSplitRelatedWindowToTop(node);
        if (isFloatWindowAboveFullWindow_ && !windowPair->IsDuringSplit()) {
            ResetAllMainFloatingWindowZOrder(appWindowNode_);
        }
    }
    MinimizeOldestMainFloatingWindow(node->GetWindowId());
    AssignZOrder();
    LayoutWhenAddWindowNode(node, afterAnimation);
    NotifyIfAvoidAreaChanged(node, AvoidControlType::AVOID_NODE_ADD);
    DumpScreenWindowTreeByWinId(node->GetWindowId());
    UpdateCameraFloatWindowStatus(node, true);
    if (WindowHelper::IsMainWindow(node->GetWindowType())) {
        backupWindowIds_.clear();
    }

    if (node->GetWindowType() == WindowType::WINDOW_TYPE_KEYGUARD) {
        isScreenLocked_ = true;
        SetBelowScreenlockVisible(node, false);
    }
    if (node->GetWindowType() == WindowType::WINDOW_TYPE_WALLPAPER) {
        RemoteAnimation::NotifyAnimationUpdateWallpaper(node);
    }
    if (node->GetWindowType() == WindowType::WINDOW_TYPE_DESKTOP) {
        DisplayManagerServiceInner::GetInstance().SetGravitySensorSubscriptionEnabled();
    }
    WLOGI("AddWindowNode Id: %{public}u end", node->GetWindowId());
    RSInterfaces::GetInstance().SetAppWindowNum(GetAppWindowNum());
    // update private window count and notify dms private status changed
    if (node->GetWindowProperty()->GetPrivacyMode()) {
        UpdatePrivateStateAndNotify();
    }
    if (WindowHelper::IsMainWindow(node->GetWindowType())) {
        WindowInfoReporter::GetInstance().InsertShowReportInfo(node->abilityInfo_.bundleName_);
    }
    return WMError::WM_OK;
}

void WindowNodeContainer::UpdateRSTreeWhenShowingDisplaysChange(sptr<WindowNode>& node,
    const std::vector<DisplayId>& lastShowingDisplays)
{
    if (!layoutPolicy_->IsMultiDisplay()) {
        return;
    }

    // Update RSTree
    auto curShowingDisplays = node->GetShowingDisplays();
    for (auto& displayId : lastShowingDisplays) {
        if (std::find(curShowingDisplays.begin(), curShowingDisplays.end(), displayId) == curShowingDisplays.end()) {
            RemoveNodeFromRSTree(node, displayId, *(curShowingDisplays.begin()),
                WindowUpdateType::WINDOW_UPDATE_ACTIVE);
            WLOGI("remove from RSTree : %{public}" PRIu64"", displayId);
        }
    }

    for (auto& displayId : curShowingDisplays) {
        if (std::find(lastShowingDisplays.begin(), lastShowingDisplays.end(), displayId) == lastShowingDisplays.end()) {
            AddNodeOnRSTree(node, displayId, displayId, WindowUpdateType::WINDOW_UPDATE_ACTIVE);
            WLOGI("add on RSTree : %{public}" PRIu64"", displayId);
        }
    }
}

WMError WindowNodeContainer::UpdateWindowNode(sptr<WindowNode>& node, WindowUpdateReason reason)
{
    // Get last displayId and last showing displays before layout
    auto lastShowingDisplays = node->GetShowingDisplays();

    // PreProcess window node and layout node
    displayGroupController_->PreProcessWindowNode(node, WindowUpdateType::WINDOW_UPDATE_ACTIVE);
    if (WindowHelper::IsMainWindow(node->GetWindowType()) && WindowHelper::IsSwitchCascadeReason(reason)) {
        SwitchLayoutPolicy(WindowLayoutMode::CASCADE, node->GetDisplayId());
    }
    layoutPolicy_->PerformWindowLayout(node, WindowUpdateType::WINDOW_UPDATE_ACTIVE);
    displayGroupController_->PostProcessWindowNode(node);
    // Get current displayId and showing displays, update RSTree and displayGroupWindowTree
    UpdateRSTreeWhenShowingDisplaysChange(node, lastShowingDisplays);
    NotifyIfAvoidAreaChanged(node, AvoidControlType::AVOID_NODE_UPDATE);
    WLOGD("UpdateNode Id: %{public}u end", node->GetWindowId());
    return WMError::WM_OK;
}

void WindowNodeContainer::RemoveWindowNodeFromWindowTree(sptr<WindowNode>& node)
{
    // remove this node from parent
    auto iter = std::find(node->parent_->children_.begin(), node->parent_->children_.end(), node);
    if (iter != node->parent_->children_.end()) {
        node->parent_->children_.erase(iter);
    } else {
        WLOGFE("can't find this node in parent");
    }
    node->parent_ = nullptr;
}

void WindowNodeContainer::RemoveFromRsTreeWhenRemoveWindowNode(sptr<WindowNode>& node, bool fromAnimation)
{
    if (fromAnimation || (RemoteAnimation::IsRemoteAnimationEnabledAndFirst(node->GetDisplayId()) &&
        node->stateMachine_.IsHideAnimationPlaying())) {
            WLOGFD("not remove from rs tree id:%{public}u", node->GetWindowId());
            return;
    }
    // When RemoteAnimation exists, remove node from rs tree after animation
    WLOGFD("Id:%{public}u, isPlayAnimationHide_:%{public}u", node->GetWindowId(),
        static_cast<uint32_t>(node->isPlayAnimationHide_));
    // subwindow or no remote animation also exit with animation
    for (auto& displayId : node->GetShowingDisplays()) {
        RemoveNodeFromRSTree(node, displayId, displayId, WindowUpdateType::WINDOW_UPDATE_REMOVED,
            node->isPlayAnimationHide_);
    }
}

void WindowNodeContainer::SetSurfaceNodeVisible(sptr<WindowNode>& node, int32_t topPriority, bool visible)
{
    if (node == nullptr) {
        return;
    }
    if (node->parent_ != nullptr && node->currentVisibility_) {
        if (node->priority_ < topPriority && !WindowHelper::IsShowWhenLocked(node->GetWindowFlags()) &&
            !WindowHelper::IsShowWhenLocked(node->parent_->GetWindowFlags())) {
            auto surfaceNode = node->leashWinSurfaceNode_ != nullptr ? node->leashWinSurfaceNode_ : node->surfaceNode_;
            if (surfaceNode) {
                surfaceNode->SetVisible(visible);
            }
        }
    }
    for (auto& childNode : node->children_) {
        SetSurfaceNodeVisible(childNode, topPriority, visible);
    }
}

void WindowNodeContainer::SetBelowScreenlockVisible(sptr<WindowNode>& node, bool visible)
{
    int32_t topPriority = zorderPolicy_->GetWindowPriority(WindowType::WINDOW_TYPE_KEYGUARD);
    std::vector<sptr<WindowNode>> rootNodes = { belowAppWindowNode_, appWindowNode_, aboveAppWindowNode_ };
    for (auto& node : rootNodes) {
        SetSurfaceNodeVisible(node, topPriority, visible);
    }
}

WMError WindowNodeContainer::RemoveWindowNode(sptr<WindowNode>& node, bool fromAnimation)
{
    if (node == nullptr) {
        WLOGFE("window node or surface node is nullptr, invalid");
        return WMError::WM_ERROR_DESTROYED_OBJECT;
    }
    if (node->parent_ == nullptr) {
        WLOGFW("can't find parent of this node");
    } else {
        RemoveWindowNodeFromWindowTree(node);
    }

    node->requestedVisibility_ = false;
    node->currentVisibility_ = false;
    RemoveFromRsTreeWhenRemoveWindowNode(node, fromAnimation);
    node->isPlayAnimationHide_ = false;
    displayGroupController_->UpdateDisplayGroupWindowTree();
    layoutPolicy_->PerformWindowLayout(node, WindowUpdateType::WINDOW_UPDATE_REMOVED);
    WindowMode lastMode = node->GetWindowMode();
    if (HandleRemoveWindow(node) != WMError::WM_OK) {
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!WindowHelper::IsFloatingWindow(lastMode)) {
        NotifyDockWindowStateChanged(node, true);
    }
    NotifyIfAvoidAreaChanged(node, AvoidControlType::AVOID_NODE_REMOVE);
    DumpScreenWindowTreeByWinId(node->GetWindowId());
    UpdateCameraFloatWindowStatus(node, false);
    if (node->GetWindowType() == WindowType::WINDOW_TYPE_KEYGUARD) {
        isScreenLocked_ = false;
        SetBelowScreenlockVisible(node, true);
    }
    WLOGI("Remove Id: %{public}u end", node->GetWindowId());
    RSInterfaces::GetInstance().SetAppWindowNum(GetAppWindowNum());

    // update private window count and notify dms private status changed
    if (node->GetWindowProperty()->GetPrivacyMode()) {
        UpdatePrivateStateAndNotify();
    }
    if (WindowHelper::IsMainWindow(node->GetWindowType())) {
        WindowInfoReporter::GetInstance().InsertHideReportInfo(node->abilityInfo_.bundleName_);
    }
    HandleRemoveWindowDisplayOrientation(node, fromAnimation);
    return WMError::WM_OK;
}

void WindowNodeContainer::HandleRemoveWindowDisplayOrientation(sptr<WindowNode>& node, bool fromAnimation)
{
    if (node->GetWindowMode() != WindowMode::WINDOW_MODE_FULLSCREEN) {
        WLOGFD("[FixOrientation] not full screen window remove, do not update orientation");
        return;
    }
    if (!FIX_ORIENTATION_ENABLE) {
        auto nextRotatableWindow = GetNextRotatableWindow(node->GetWindowId());
        if (nextRotatableWindow != nullptr) {
            SetDisplayOrientationFromWindow(nextRotatableWindow, true);
        }
        return;
    }
    if (!fromAnimation) {
        if (node->stateMachine_.IsHideAnimationPlaying()) {
            WLOGFD("[FixOrientation] removing window is playing hide animation, do not update display orientation");
            return;
        }
        auto nextRotatableWindow = GetNextRotatableWindow(node->GetWindowId());
        if (nextRotatableWindow == nullptr) {
            WLOGFD("[FixOrientation] no next window, do not update display orientation");
            return;
        }
        WLOGFD("[FixOrientation] nexi rotatable window: %{public}u", nextRotatableWindow->GetWindowId());
        if (nextRotatableWindow->stateMachine_.IsShowAnimationPlaying()) {
            WLOGFD("[FixOrientation] next window is playing show animation, do not update display orientation");
            return;
        }
        if (WmsUtils::IsFixedOrientation(nextRotatableWindow->GetRequestedOrientation(),
            nextRotatableWindow->GetWindowMode(), nextRotatableWindow->GetWindowFlags())) {
            WLOGFI("[FixOrientation] next rotatable window is fixed, do not animation");
            SetDisplayOrientationFromWindow(nextRotatableWindow, false);
        } else {
            SetDisplayOrientationFromWindow(nextRotatableWindow, true);
        }
    }
}

void WindowNodeContainer::SetDisplayOrientationFromWindow(sptr<WindowNode>& node, bool withAnimation)
{
    DisplayManagerServiceInner::GetInstance().SetOrientationFromWindow(node->GetDisplayId(),
        node->GetRequestedOrientation(), withAnimation);
}

void WindowNodeContainer::UpdatePrivateStateAndNotify()
{
    uint32_t prePrivateWindowCount = privateWindowCount_;
    WLOGFD("before update : privateWindow count: %{public}u", prePrivateWindowCount);
    UpdatePrivateWindowCount();
    if (prePrivateWindowCount == 0 && privateWindowCount_ == 1) {
        DisplayManagerServiceInner::GetInstance().NotifyPrivateWindowStateChanged(true);
    } else if (prePrivateWindowCount == 1 && privateWindowCount_ == 0) {
        DisplayManagerServiceInner::GetInstance().NotifyPrivateWindowStateChanged(false);
    }
}

void WindowNodeContainer::UpdatePrivateWindowCount()
{
    std::vector<sptr<WindowNode>> windowNodes;
    TraverseContainer(windowNodes);
    uint32_t count = 0;
    for (const auto& node : windowNodes) {
        if (node->GetWindowProperty()->GetPrivacyMode()) {
            ++count;
        }
    }
    privateWindowCount_ = count;
    WLOGFD("after update : privateWindow count: %{public}u", privateWindowCount_);
}

uint32_t WindowNodeContainer::GetAppWindowNum()
{
    uint32_t num = 0;
    for (auto& child : appWindowNode_->children_) {
        if (WindowHelper::IsAppWindow(child->GetWindowType())) {
            num++;
        }
    }
    return num;
}

void WindowNodeContainer::SetConfigMainFloatingWindowAbove(bool isAbove)
{
    isFloatWindowAboveFullWindow_ = isAbove;
}

void WindowNodeContainer::SetMaxMainFloatingWindowNumber(uint32_t maxNumber)
{
    maxMainFloatingWindowNumber_ = maxNumber;
}

void WindowNodeContainer::ResetMainFloatingWindowPriorityIfNeeded(sptr<WindowNode>& windowNode)
{
    if (!isFloatWindowAboveFullWindow_) {
        return;
    }
    const WindowType& windowType = windowNode->GetWindowType();
    const WindowMode& windowMode = windowNode->GetWindowMode();
    if (!WindowHelper::IsMainFloatingWindow(windowType, windowMode)) {
        return;
    }
    const int32_t priorityOffset = 1;
    auto baseZOrderPolicy = zorderPolicy_->GetWindowPriority(windowType);
    if (isScreenLocked_ &&
        (windowNode->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED))) {
        // if window show when lock, priority should bigger than KEYGUARD.
        baseZOrderPolicy = zorderPolicy_->GetWindowPriority(WindowType::WINDOW_TYPE_KEYGUARD) + 1;
    }

    auto windowPair = displayGroupController_->GetWindowPairByDisplayId(windowNode->GetDisplayId());
    if (windowPair != nullptr && windowPair->IsDuringSplit()) {
        windowNode->priority_ = baseZOrderPolicy - priorityOffset;
        WLOGFD("Reset floating main window zorder priority.[windowId: %{public}u, priority: %{public}d] ",
            windowNode->GetWindowId(), windowNode->priority_);
        return;
    }

    windowNode->priority_ = baseZOrderPolicy + priorityOffset;
    WLOGFD("Reset floating main window zorder priority.[windowId: %{public}u, priority: %{public}d] ",
        windowNode->GetWindowId(), windowNode->priority_);
}

void WindowNodeContainer::ResetAllMainFloatingWindowZOrder(sptr<WindowNode>& rootNode)
{
    if (!isFloatWindowAboveFullWindow_) {
        WLOGFD("The free window level above full screen window feature is turned off");
        return;
    }
    if (rootNode != appWindowNode_ && rootNode != aboveAppWindowNode_) {
        return;
    }
    /*
     * update all mainFloatingWindow position on window tree with
     * the same raleative position between mainFloatingWindows.
     */
    std::vector<sptr<WindowNode>> tempWindows;
    auto itor = rootNode->children_.begin();
    while (itor != rootNode->children_.end()) {
        const WindowType& windowType = (*itor)->GetWindowType();
        const WindowMode& windowMode = (*itor)->GetWindowMode();
        if (WindowHelper::IsMainFloatingWindow(windowType, windowMode)) {
            tempWindows.push_back(*itor);
            itor = rootNode->children_.erase(itor);
        } else {
            itor++;
        }
    }

    for (auto& node : tempWindows) {
        UpdateWindowTree(node);
    }
}

WMError WindowNodeContainer::HandleRemoveWindow(sptr<WindowNode>& node)
{
    WLOGFD("start");
    auto windowPair = displayGroupController_->GetWindowPairByDisplayId(node->GetDisplayId());
    if (windowPair == nullptr) {
        WLOGFE("Window pair is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    windowPair->HandleRemoveWindow(node);
    auto dividerWindow = windowPair->GetDividerWindow();
    auto type = node->GetWindowType();
    if ((type == WindowType::WINDOW_TYPE_STATUS_BAR || type == WindowType::WINDOW_TYPE_NAVIGATION_BAR) &&
        dividerWindow != nullptr) {
        UpdateWindowNode(dividerWindow, WindowUpdateReason::UPDATE_RECT);
    }
    WLOGFD("end");
    return WMError::WM_OK;
}

WMError WindowNodeContainer::DestroyWindowNode(sptr<WindowNode>& node, std::vector<uint32_t>& windowIds)
{
    WMError ret = RemoveWindowNode(node);
    if (ret != WMError::WM_OK) {
        WLOGFE("RemoveWindowNode failed");
        return ret;
    }
    StartingWindow::ReleaseStartWinSurfaceNode(node);
    node->surfaceNode_ = nullptr;
    windowIds.push_back(node->GetWindowId());
    for (auto& child : node->children_) { // destroy sub window if exists
        windowIds.push_back(child->GetWindowId());
        child->parent_ = nullptr;
        if (child->surfaceNode_ != nullptr) {
            WLOGI("child surfaceNode set nullptr");
            child->surfaceNode_ = nullptr;
        }
    }

    // clear vector cache completely, swap with empty vector
    auto emptyVector = std::vector<sptr<WindowNode>>();
    node->children_.swap(emptyVector);
    if (node->GetWindowType() == WindowType::WINDOW_TYPE_WALLPAPER) {
        RemoteAnimation::NotifyAnimationUpdateWallpaper(nullptr);
    }
    if (WindowHelper::IsMainWindow(node->GetWindowType())) {
        WindowInfoReporter::GetInstance().InsertDestroyReportInfo(node->abilityInfo_.bundleName_);
    }

    UpdateAvoidAreaListener(node, false);
    WLOGI("DestroyNode Id: %{public}u end", node->GetWindowId());
    return WMError::WM_OK;
}

void WindowNodeContainer::UpdateSizeChangeReason(sptr<WindowNode>& node, WindowSizeChangeReason reason)
{
    if (!node->GetWindowToken()) {
        WLOGFE("windowToken is null");
        return;
    }
    if (node->GetWindowType() == WindowType::WINDOW_TYPE_DOCK_SLICE) {
        for (auto& child : appWindowNode_->children_) {
            if (child->IsSplitMode() && child->GetWindowToken()) {
                layoutPolicy_->NotifyClientAndAnimation(child, child->GetWindowRect(), reason);
                WLOGI("Notify split window that drag is start or end, Id: "
                    "%{public}d, reason: %{public}u", child->GetWindowId(), reason);
            }
        }
    } else {
        layoutPolicy_->NotifyClientAndAnimation(node, node->GetWindowRect(), reason);
        WLOGI("Notify window that drag is start or end, windowId: %{public}d, "
            "reason: %{public}u", node->GetWindowId(), reason);
    }
}

void WindowNodeContainer::UpdateWindowTree(sptr<WindowNode>& node)
{
    HITRACE_METER(HITRACE_TAG_WINDOW_MANAGER);
    node->priority_ = zorderPolicy_->GetWindowPriority(node->GetWindowType());
    RaiseInputMethodWindowPriorityIfNeeded(node);
    RaiseShowWhenLockedWindowIfNeeded(node);
    ResetMainFloatingWindowPriorityIfNeeded(node);
    auto parentNode = node->parent_;
    if (parentNode == nullptr) {
        WLOGI("Current window node has no parent: %{public}u", node->GetWindowId());
        return;
    }
    auto iter = std::find(parentNode->children_.begin(), parentNode->children_.end(), node);
    if (iter != parentNode->children_.end()) {
        WLOGI("node %{public}u already on window tree, not update!", node->GetWindowId());
        return;
    }
    auto position = parentNode->children_.end();
    int splitWindowCnt = 0;
    for (auto child = parentNode->children_.begin(); child < parentNode->children_.end(); ++child) {
        if (node->GetWindowType() == WindowType::WINDOW_TYPE_DOCK_SLICE && splitWindowCnt == SPLIT_WINDOWS_CNT) {
            position = child;
            break;
        }
        if (WindowHelper::IsSplitWindowMode((*child)->GetWindowMode())) {
            splitWindowCnt++;
        }
        if ((*child)->priority_ > node->priority_) {
            position = child;
            break;
        }
    }
    parentNode->children_.insert(position, node);
}

bool WindowNodeContainer::AddAppSurfaceNodeOnRSTree(sptr<WindowNode>& node)
{
    /*
     * App main window must has starting window, and show after starting window
     * Starting Window has already update leashWindowSurfaceNode and starting window surfaceNode on RSTree
     * Just need add appSurface Node as child of leashWindowSurfaceNode
     */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "AddAppSurfaceNodeOnRSTree(%u)", node->GetWindowId());
    if (!WindowHelper::IsMainWindow(node->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "id:%{public}u not main app window but has start window",
            node->GetWindowId());
        return false;
    }
    if (!node->leashWinSurfaceNode_ || !node->surfaceNode_) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "id:%{public}u leashWinSurfaceNode or surfaceNode is null "
            "but has start window!", node->GetWindowId());
        return false;
    }
    TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "Id: %{public}d", node->GetWindowId());
    if (!node->currentVisibility_) {
        TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "id: %{public}d is invisible, no need update RS tree",
            node->GetWindowId());
        return false;
    }
    node->leashWinSurfaceNode_->AddChild(node->surfaceNode_, -1);
    return true;
}

void WindowNodeContainer::RunInInputMethodSyncTransaction(sptr<WindowNode>& node, std::function<void()> task)
{
    if (isAnimateTransactionEnabled_) {
        // Use AutoRSSyncTransaction to automatically manage the Open and Close of RSSyncTransaction,
        // and perform FlushImplicitTransaction when necessary.
        AutoRSSyncTransaction autoSync(node->GetRSUIContext(), true);
        task();
    } else {
        TLOGD(WmsLogTag::DEFAULT, "InputMethodSyncTransaction is disabled");
        task();
    }
}

bool WindowNodeContainer::IsWindowFollowParent(WindowType type)
{
    if (windowUIType_ != WindowUIType::PHONE_WINDOW) {
        return false;
    }
    return WindowHelper::IsWindowFollowParent(type);
}

bool WindowNodeContainer::AddNodeOnRSTree(sptr<WindowNode>& node, DisplayId displayId, DisplayId parentDisplayId,
    WindowUpdateType type, bool animationPlayed)
{
    HITRACE_METER(HITRACE_TAG_WINDOW_MANAGER);
    if (node->GetWindowProperty()->GetAnimationFlag() == static_cast<uint32_t>(WindowAnimation::CUSTOM) ||
        node->GetWindowType() == WindowType::WINDOW_TYPE_APP_COMPONENT) {
        WLOGI("no need to update RSTree");
        return true;
    }
    bool isMultiDisplay = layoutPolicy_->IsMultiDisplay();
    WLOGFD("Id: %{public}d, displayId: %{public}" PRIu64", parentDisplayId: %{public}" PRIu64", "
        "isMultiDisplay: %{public}d, animationPlayed: %{public}d",
        node->GetWindowId(), displayId, parentDisplayId, isMultiDisplay, animationPlayed);
    auto updateRSTreeFunc = [&]() {
        if (!node->currentVisibility_) {
            WLOGI("id: %{public}d invisible, no need update RS tree", node->GetWindowId());
            return;
        }

        if (IsWindowFollowParent(node->GetWindowType())) {
            auto& parentNode = node->parent_;
            if (parentNode != nullptr && parentNode->surfaceNode_ != nullptr &&
                node->surfaceNode_ != nullptr) {
                node->surfaceNode_->SetTranslateX(node->GetWindowRect().posX_ - parentNode->GetWindowRect().posX_);
                node->surfaceNode_->SetTranslateY(node->GetWindowRect().posY_ - parentNode->GetWindowRect().posY_);
                node->surfaceNode_->SetVisible(true);
                parentNode->surfaceNode_->AddChild(node->surfaceNode_, -1);
                WLOGFD("Add surfaceNode to parent surfaceNode succeed.");
                return;
            }
        }
        auto& dms = DisplayManagerServiceInner::GetInstance();
        auto& surfaceNode = node->leashWinSurfaceNode_ != nullptr ? node->leashWinSurfaceNode_ : node->surfaceNode_;
        dms.UpdateRSTree(displayId, parentDisplayId, surfaceNode, true, isMultiDisplay);
        for (auto& child : node->children_) {
            if (child->currentVisibility_ && !IsWindowFollowParent(child->GetWindowType())) {
                dms.UpdateRSTree(displayId, parentDisplayId, child->surfaceNode_, true, isMultiDisplay);
            }
        }
    };

    if (type != WindowUpdateType::WINDOW_UPDATE_ADDED && type != WindowUpdateType::WINDOW_UPDATE_REMOVED) {
        updateRSTreeFunc();
        return true;
    }

    WindowGravity windowGravity;
    uint32_t percent;
    node->GetWindowGravity(windowGravity, percent);

    auto rsUIContext = node->GetRSUIContext();
    if (node->EnableDefaultAnimation(animationPlayed)) {
        WLOGFD("Add node with animation");
        StartTraceArgs(HITRACE_TAG_WINDOW_MANAGER, "Animate(%u)", node->GetWindowId());
        RSNode::Animate(rsUIContext, animationConfig_.windowAnimationConfig_.animationTiming_.timingProtocol_,
            animationConfig_.windowAnimationConfig_.animationTiming_.timingCurve_, updateRSTreeFunc);
        FinishTrace(HITRACE_TAG_WINDOW_MANAGER);
    } else if (node->GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT &&
        windowGravity != WindowGravity::WINDOW_GRAVITY_FLOAT &&
        !animationPlayed) { // add keyboard with animation
        auto timingProtocol = animationConfig_.keyboardAnimationIn_.duration_;
        RunInInputMethodSyncTransaction(node, [&] {
            RSNode::Animate(
                rsUIContext, timingProtocol, animationConfig_.keyboardAnimationIn_.curve_, updateRSTreeFunc);
        });
    } else {
        WLOGFD("add node without animation");
        updateRSTreeFunc();
    }
    return true;
}

bool WindowNodeContainer::RemoveNodeFromRSTree(sptr<WindowNode>& node, DisplayId displayId, DisplayId parentDisplayId,
    WindowUpdateType type, bool animationPlayed)
{
    HITRACE_METER(HITRACE_TAG_WINDOW_MANAGER);
    if (node->GetWindowProperty()->GetAnimationFlag() == static_cast<uint32_t>(WindowAnimation::CUSTOM) ||
        node->GetWindowType() == WindowType::WINDOW_TYPE_APP_COMPONENT) {
        WLOGI("no need to update RSTree");
        return true;
    }
    bool isMultiDisplay = layoutPolicy_->IsMultiDisplay();
    WLOGFD("Id: %{public}d, displayId: %{public}" PRIu64", isMultiDisplay: %{public}d, "
        "parentDisplayId: %{public}" PRIu64", animationPlayed: %{public}d",
        node->GetWindowId(), displayId, isMultiDisplay, parentDisplayId, animationPlayed);
    auto updateRSTreeFunc = [&]() {
        if (IsWindowFollowParent(node->GetWindowType())) {
            const auto& parentNode = node->parent_;
            if (parentNode != nullptr && parentNode->surfaceNode_ != nullptr &&
                node->surfaceNode_ != nullptr) {
                node->surfaceNode_->SetVisible(false);
                parentNode->surfaceNode_->RemoveChild(node->surfaceNode_);
                WLOGFD("Remove surfaceNode to parent surfaceNode succeed.");
                return;
            }
        }
        auto& dms = DisplayManagerServiceInner::GetInstance();
        auto& surfaceNode = node->leashWinSurfaceNode_ != nullptr ? node->leashWinSurfaceNode_ : node->surfaceNode_;
        dms.UpdateRSTree(displayId, parentDisplayId, surfaceNode, false, isMultiDisplay);
        for (auto& child : node->children_) {
            if (child->currentVisibility_ && !IsWindowFollowParent(child->GetWindowType())) {
                dms.UpdateRSTree(displayId, parentDisplayId, child->surfaceNode_, false, isMultiDisplay);
            }
        }
    };

    if (type != WindowUpdateType::WINDOW_UPDATE_ADDED && type != WindowUpdateType::WINDOW_UPDATE_REMOVED) {
        updateRSTreeFunc();
        return true;
    }

    WindowGravity windowGravity;
    uint32_t percent;
    node->GetWindowGravity(windowGravity, percent);

    auto rsUIContext = node->GetRSUIContext();
    if (node->EnableDefaultAnimation(animationPlayed)) {
        WLOGFD("remove with animation");
        StartTraceArgs(HITRACE_TAG_WINDOW_MANAGER, "Animate(%u)", node->GetWindowId());
        if (node->surfaceNode_) {
            node->surfaceNode_->SetFreeze(true);
        }
        wptr<WindowNode> weakNode(node);
        RSNode::Animate(rsUIContext, animationConfig_.windowAnimationConfig_.animationTiming_.timingProtocol_,
            animationConfig_.windowAnimationConfig_.animationTiming_.timingCurve_, updateRSTreeFunc, [weakNode]() {
            auto weakWindow = weakNode.promote();
            if (weakWindow && weakWindow->surfaceNode_) {
                weakWindow->surfaceNode_->SetFreeze(false);
            }
        });
        FinishTrace(HITRACE_TAG_WINDOW_MANAGER);
    } else if (node->GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT &&
        windowGravity != WindowGravity::WINDOW_GRAVITY_FLOAT && !animationPlayed) {
        // remove keyboard with animation
        RunInInputMethodSyncTransaction(node, [&] {
            auto timingProtocol = animationConfig_.keyboardAnimationOut_.duration_;
            RSNode::Animate(
                rsUIContext, timingProtocol, animationConfig_.keyboardAnimationOut_.curve_, updateRSTreeFunc);
        });
    } else {
        updateRSTreeFunc();
    }
    return true;
}

const std::vector<uint32_t>& WindowNodeContainer::Destroy()
{
    // clear vector cache completely, swap with empty vector
    auto emptyVector = std::vector<uint32_t>();
    removedIds_.swap(emptyVector);
    for (auto& node : belowAppWindowNode_->children_) {
        DestroyWindowNode(node, removedIds_);
    }
    for (auto& node : appWindowNode_->children_) {
        DestroyWindowNode(node, removedIds_);
    }
    for (auto& node : aboveAppWindowNode_->children_) {
        DestroyWindowNode(node, removedIds_);
    }
    return removedIds_;
}

sptr<WindowNode> WindowNodeContainer::FindRoot(WindowType type) const
{
    if (WindowHelper::IsAppWindow(type) || type == WindowType::WINDOW_TYPE_DOCK_SLICE ||
        type == WindowType::WINDOW_TYPE_APP_COMPONENT || type == WindowType::WINDOW_TYPE_PLACEHOLDER ||
        type == WindowType::WINDOW_TYPE_DIALOG) {
        return appWindowNode_;
    }
    if (WindowHelper::IsBelowSystemWindow(type)) {
        return belowAppWindowNode_;
    }
    if (WindowHelper::IsAboveSystemWindow(type)) {
        return aboveAppWindowNode_;
    }
    return nullptr;
}

sptr<WindowNode> WindowNodeContainer::FindWindowNodeById(uint32_t id) const
{
    std::vector<sptr<WindowNode>> rootNodes = { aboveAppWindowNode_, appWindowNode_, belowAppWindowNode_ };
    for (const auto& rootNode : rootNodes) {
        for (auto& node : rootNode->children_) {
            if (node->GetWindowId() == id) {
                return node;
            }
            for (auto& subNode : node->children_) {
                if (subNode->GetWindowId() == id) {
                    return subNode;
                }
            }
        }
    }
    return nullptr;
}

void WindowNodeContainer::UpdateFocusStatus(uint32_t id, bool focused)
{
    auto node = FindWindowNodeById(id);
    if (node == nullptr) {
        WLOGFW("cannot find focused window id:%{public}d", id);
        return;
    }
    if (focused) {
        focusedPid_ = node->GetCallingPid();
    }
    node->isFocused_ = focused;
    // change focus window shadow
    WindowSystemEffect::SetWindowShadow(node);
    if (node->GetCallingPid() == 0) {
        WLOGFW("focused window is starting window, no need notify");
        return;
    }

    if (focused && node->GetWindowProperty() != nullptr) {
        AbilityInfo info = node->GetWindowProperty()->GetAbilityInfo();
        WLOGFD("current focus window: windowId: %{public}d, windowName: %{public}s, bundleName: %{public}s,"
            " abilityName: %{public}s, pid: %{public}d, uid: %{public}d", id,
            node->GetWindowProperty()->GetWindowName().c_str(), info.bundleName_.c_str(), info.abilityName_.c_str(),
            node->GetCallingPid(), node->GetCallingUid());
        uint64_t focusNodeId = 0; // 0 means invalid
        if (node->surfaceNode_ == nullptr) {
            WLOGFW("focused window surfaceNode is null");
        } else {
            focusNodeId = node->surfaceNode_->GetId();
        }
        FocusAppInfo appInfo =
            { node->GetCallingPid(), node->GetCallingUid(), info.bundleName_, info.abilityName_, focusNodeId };
        RSInterfaces::GetInstance().SetFocusAppInfo(appInfo);
    }
    if (node->GetWindowToken()) {
        node->GetWindowToken()->UpdateFocusStatus(focused);
    }
    if (node->abilityToken_ == nullptr) {
        WLOGW("AbilityToken is null, window : %{public}d", id);
    }
    sptr<FocusChangeInfo> focusChangeInfo = new FocusChangeInfo(node->GetWindowId(), node->GetDisplayId(),
        node->GetDisplayId(), DISPLAY_GROUP_ID_DEFAULT, node->GetCallingPid(), node->GetCallingUid(),
        node->GetWindowType(), node->abilityToken_);
    WindowManagerAgentController::GetInstance().UpdateFocusChangeInfo(
        focusChangeInfo, focused);
}

void WindowNodeContainer::UpdateActiveStatus(uint32_t id, bool isActive)
{
    auto node = FindWindowNodeById(id);
    if (node == nullptr) {
        WLOGFE("cannot find active window id: %{public}d", id);
        return;
    }
    if (isActive) {
        activePid_ = node->GetCallingPid();
    }
    if (node->GetWindowToken()) {
        node->GetWindowToken()->UpdateActiveStatus(isActive);
    }
}

void WindowNodeContainer::UpdateBrightness(uint32_t id, bool byRemoved)
{
    auto node = FindWindowNodeById(id);
    if (node == nullptr) {
        WLOGFE("cannot find active window id: %{public}d", id);
        return;
    }

    if (!byRemoved) {
        if (!WindowHelper::IsAppWindow(node->GetWindowType())) {
            return;
        }
    }
    WLOGI("Brightness: [%{public}f, %{public}f]", GetDisplayBrightness(), node->GetBrightness());
    if (std::fabs(node->GetBrightness() - UNDEFINED_BRIGHTNESS) < std::numeric_limits<float>::min()) {
        if (GetDisplayBrightness() != node->GetBrightness()) {
            WLOGI("adjust brightness with default value");
#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
            DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().RestoreBrightness();
#endif
            SetDisplayBrightness(UNDEFINED_BRIGHTNESS); // UNDEFINED_BRIGHTNESS means system default brightness
        }
        SetBrightnessWindow(INVALID_WINDOW_ID);
    } else {
        if (GetDisplayBrightness() != node->GetBrightness()) {
            WLOGI("adjust brightness with value: %{public}u", ToOverrideBrightness(node->GetBrightness()));
#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
            DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().OverrideBrightness(
                ToOverrideBrightness(node->GetBrightness()));
#endif
            SetDisplayBrightness(node->GetBrightness());
        }
        SetBrightnessWindow(node->GetWindowId());
    }
}

void WindowNodeContainer::AssignZOrder()
{
    zOrder_ = 0;
    WindowNodeOperationFunc func = [this](sptr<WindowNode> node) {
        if (!node->leashWinSurfaceNode_ && !node->surfaceNode_ && !node->startingWinSurfaceNode_) {
            ++zOrder_;
            WLOGFE("Id: %{public}u has no surface nodes", node->GetWindowId());
            return false;
        }
        if (node->leashWinSurfaceNode_ != nullptr) {
            ++zOrder_;
            node->leashWinSurfaceNode_->SetPositionZ(zOrder_);
        }

        if (node->surfaceNode_ != nullptr) {
            ++zOrder_;
            node->surfaceNode_->SetPositionZ(zOrder_);
            node->zOrder_ = zOrder_;
        }
        // make sure starting window above app
        if (node->startingWinSurfaceNode_ != nullptr) {
            ++zOrder_;
            node->startingWinSurfaceNode_->SetPositionZ(zOrder_);
        }
        return false;
    };
    TraverseWindowTree(func, false);
    displayGroupController_->UpdateDisplayGroupWindowTree();
}

WMError WindowNodeContainer::SetFocusWindow(uint32_t windowId)
{
    if (focusedWindow_ == windowId) {
        WLOGI("Focus window not change, id: %{public}u, %{public}d", windowId, focusedPid_);
        // StartingWindow can be focused and this pid is 0, then notify info in UpdateFocusStatus.
        // This info is invalid, so we must notify again when first frame callback.
        if (focusedPid_ == 0) {
            UpdateFocusStatus(windowId, true);
        }
        return WMError::WM_DO_NOTHING;
    }
    UpdateFocusStatus(focusedWindow_, false);
    focusedWindow_ = windowId;
    UpdateFocusStatus(focusedWindow_, true);
    return WMError::WM_OK;
}

uint32_t WindowNodeContainer::GetFocusWindow() const
{
    return focusedWindow_;
}

WMError WindowNodeContainer::SetActiveWindow(uint32_t windowId, bool byRemoved)
{
    if (activeWindow_ == windowId) {
        WLOGI("Active window not change, id: %{public}u, %{public}d", windowId, activePid_);
        if (activePid_ == 0) {
            UpdateActiveStatus(windowId, true);
        }
        return WMError::WM_DO_NOTHING;
    }
    UpdateActiveStatus(activeWindow_, false);
    activeWindow_ = windowId;
    UpdateActiveStatus(activeWindow_, true);
    UpdateBrightness(activeWindow_, byRemoved);
    return WMError::WM_OK;
}

void WindowNodeContainer::SetDisplayBrightness(float brightness)
{
    displayBrightness_ = brightness;
}

float WindowNodeContainer::GetDisplayBrightness() const
{
    return displayBrightness_;
}

void WindowNodeContainer::SetBrightnessWindow(uint32_t windowId)
{
    brightnessWindow_ = windowId;
}

uint32_t WindowNodeContainer::GetBrightnessWindow() const
{
    return brightnessWindow_;
}

uint32_t WindowNodeContainer::ToOverrideBrightness(float brightness)
{
    return static_cast<uint32_t>(brightness * MAX_BRIGHTNESS);
}

uint32_t WindowNodeContainer::GetActiveWindow() const
{
    return activeWindow_;
}

sptr<WindowLayoutPolicy> WindowNodeContainer::GetLayoutPolicy() const
{
    return layoutPolicy_;
}

sptr<AvoidAreaController> WindowNodeContainer::GetAvoidController() const
{
    return avoidController_;
}

sptr<DisplayGroupController> WindowNodeContainer::GetDisplayGroupController() const
{
    return displayGroupController_;
}

sptr<WindowNode> WindowNodeContainer::GetRootNode(WindowRootNodeType type) const
{
    if (type == WindowRootNodeType::ABOVE_WINDOW_NODE) {
        return aboveAppWindowNode_;
    } else if (type == WindowRootNodeType::APP_WINDOW_NODE) {
        return appWindowNode_;
    } else if (type == WindowRootNodeType::BELOW_WINDOW_NODE) {
        return belowAppWindowNode_;
    }
    return nullptr;
}

void WindowNodeContainer::HandleKeepScreenOn(const sptr<WindowNode>& node, bool requireLock)
{
#ifdef POWER_MANAGER_ENABLE
    if (requireLock && node->keepScreenLock_ == nullptr) {
        // reset ipc identity
        std::string identity = IPCSkeleton::ResetCallingIdentity();
        node->keepScreenLock_ = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock(node->GetWindowName(),
            PowerMgr::RunningLockType::RUNNINGLOCK_SCREEN);
        // set ipc identity to raw
        IPCSkeleton::SetCallingIdentity(identity);
    }
    if (node->keepScreenLock_ == nullptr) {
        return;
    }
    WLOGI("keep screen on: [%{public}s, %{public}d]", node->GetWindowName().c_str(), requireLock);
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "container:HandleKeepScreenOn(%s, %d)",
        node->GetWindowName().c_str(), requireLock);
    ErrCode res;
    // reset ipc identity
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    if (requireLock) {
        res = node->keepScreenLock_->Lock();
    } else {
        res = node->keepScreenLock_->UnLock();
    }
    // set ipc identity to raw
    IPCSkeleton::SetCallingIdentity(identity);
    if (res != ERR_OK) {
        WLOGFE("handle keep screen running lock failed: [operation: %{public}d, err: %{public}d]", requireLock, res);
    }
#endif
}

bool WindowNodeContainer::IsAboveSystemBarNode(sptr<WindowNode> node) const
{
    int32_t curPriority = zorderPolicy_->GetWindowPriority(node->GetWindowType());
    if ((curPriority > zorderPolicy_->GetWindowPriority(WindowType::WINDOW_TYPE_STATUS_BAR)) &&
        (curPriority > zorderPolicy_->GetWindowPriority(WindowType::WINDOW_TYPE_NAVIGATION_BAR))) {
        return true;
    }
    return false;
}

bool WindowNodeContainer::IsSplitImmersiveNode(sptr<WindowNode> node) const
{
    auto type = node->GetWindowType();
    return node->IsSplitMode() || type == WindowType::WINDOW_TYPE_DOCK_SLICE;
}

std::unordered_map<WindowType, SystemBarProperty> WindowNodeContainer::GetExpectImmersiveProperty(DisplayId id,
    sptr<WindowNode>& triggerWindow) const
{
    std::unordered_map<WindowType, SystemBarProperty> sysBarPropMap {
        { WindowType::WINDOW_TYPE_STATUS_BAR,     SystemBarProperty() },
        { WindowType::WINDOW_TYPE_NAVIGATION_BAR, SystemBarProperty() },
    };

    std::vector<sptr<WindowNode>> rootNodes = { aboveAppWindowNode_, appWindowNode_, belowAppWindowNode_ };
    if (layoutMode_ == WindowLayoutMode::TILE) {
        rootNodes = { aboveAppWindowNode_, belowAppWindowNode_ };
    }

    for (const auto& node : rootNodes) {
        for (auto iter = node->children_.rbegin(); iter < node->children_.rend(); ++iter) {
            auto& sysBarPropMapNode = (*iter)->GetSystemBarProperty();
            if (IsAboveSystemBarNode(*iter)) {
                continue;
            }
            if (WindowHelper::IsFullScreenWindow((*iter)->GetWindowMode())
                && (*iter)->GetWindowType() != WindowType::WINDOW_TYPE_PANEL) {
                auto displayInfo = DisplayGroupInfo::GetInstance().GetDisplayInfo(id);
                if (displayInfo && WmsUtils::IsExpectedRotateLandscapeWindow((*iter)->GetRequestedOrientation(),
                    displayInfo->GetDisplayOrientation(), (*iter)->GetWindowFlags())) {
                    WLOGFI("Horizontal window id: %{public}d make it immersive", (*iter)->GetWindowId());
                    for (auto it : sysBarPropMap) {
                        sysBarPropMap[it.first] = (sysBarPropMapNode.find(it.first))->second;
                        sysBarPropMap[it.first].enable_ = false;
                    }
                } else {
                    WLOGFD("Top immersive window id: %{public}d. Use full immersive prop", (*iter)->GetWindowId());
                    for (auto it : sysBarPropMap) {
                        sysBarPropMap[it.first] = (sysBarPropMapNode.find(it.first))->second;
                    }
                    triggerWindow = (*iter);
                }
                return sysBarPropMap;
            } else if (IsSplitImmersiveNode(*iter)) {
                WLOGFD("Top split window id: %{public}d. Use split immersive prop", (*iter)->GetWindowId());
                for (auto it : sysBarPropMap) {
                    sysBarPropMap[it.first] = (sysBarPropMapNode.find(it.first))->second;
                    sysBarPropMap[it.first].enable_ = false;
                }
                return sysBarPropMap;
            }
        }
    }

    WLOGFD("No immersive window on top. Use default systembar Property");
    return sysBarPropMap;
}

void WindowNodeContainer::NotifyIfAvoidAreaChanged(const sptr<WindowNode>& node,
    const AvoidControlType avoidType) const
{
    auto checkFunc = [this](sptr<WindowNode> node) {
        return CheckWindowNodeWhetherInWindowTree(node);
    };
    avoidController_->ProcessWindowChange(node, avoidType, checkFunc);
    if (WindowHelper::IsSystemBarWindow(node->GetWindowType())) {
        NotifyIfSystemBarRegionChanged(node->GetDisplayId());
    } else {
        NotifyIfSystemBarTintChanged(node->GetDisplayId());
    }

    NotifyIfKeyboardRegionChanged(node, avoidType);
}

void WindowNodeContainer::BeforeProcessWindowAvoidAreaChangeWhenDisplayChange() const
{
    avoidController_->SetFlagForProcessWindowChange(true);
}

void WindowNodeContainer::ProcessWindowAvoidAreaChangeWhenDisplayChange() const
{
    avoidController_->SetFlagForProcessWindowChange(false);
    auto checkFunc = [this](sptr<WindowNode> node) {
        return CheckWindowNodeWhetherInWindowTree(node);
    };
    WindowNodeOperationFunc func = [avoidController = avoidController_, &checkFunc](sptr<WindowNode> node) {
        avoidController->ProcessWindowChange(node, AvoidControlType::AVOID_NODE_UPDATE, checkFunc);
        return false;
    };
    TraverseWindowTree(func, true);
}

void WindowNodeContainer::NotifyIfSystemBarTintChanged(DisplayId displayId) const
{
    HITRACE_METER(HITRACE_TAG_WINDOW_MANAGER);
    sptr<WindowNode> triggerWindow = nullptr;
    auto expectSystemBarProp = GetExpectImmersiveProperty(displayId, triggerWindow);
    JudgeToReportSystemBarInfo(triggerWindow, expectSystemBarProp);
    SystemBarRegionTints tints;
    SysBarTintMap& sysBarTintMap = displayGroupController_->sysBarTintMaps_[displayId];
    for (auto it : sysBarTintMap) {
        auto expectProp = expectSystemBarProp.find(it.first)->second;
        if (it.second.prop_ == expectProp) {
            continue;
        }
        WLOGFD("System bar prop update, Type: %{public}d, Visible: %{public}d, Color: %{public}x | %{public}x",
            static_cast<int32_t>(it.first), expectProp.enable_, expectProp.backgroundColor_, expectProp.contentColor_);
        sysBarTintMap[it.first].prop_ = expectProp;
        sysBarTintMap[it.first].type_ = it.first;
        tints.emplace_back(sysBarTintMap[it.first]);
    }
    WindowManagerAgentController::GetInstance().UpdateSystemBarRegionTints(displayId, tints);
}

void WindowNodeContainer::JudgeToReportSystemBarInfo(const sptr<WindowNode> window,
    const std::unordered_map<WindowType, SystemBarProperty>& systemBarPropInfo) const
{
    if (window == nullptr || !WindowHelper::IsMainWindow(window->GetWindowType())) {
        WLOGFD("No need to report");
        return;
    }

    // 2 means the must size of systemBarPropInfo.
    if (systemBarPropInfo.size() != 2) {
        return;
    }

    auto bundleName = window->abilityInfo_.bundleName_;
    auto abilityName = window->abilityInfo_.abilityName_;
    auto navigationItor = systemBarPropInfo.find(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    if (navigationItor != systemBarPropInfo.end() && !navigationItor->second.enable_) {
        WindowInfoReporter::GetInstance().InsertNavigationBarReportInfo(bundleName, abilityName);
        WLOGFD("the navigation bar is disabled by window. windowId:[%{public}u]", window->GetWindowId());
    }
}

void WindowNodeContainer::NotifyIfSystemBarRegionChanged(DisplayId displayId) const
{
    HITRACE_METER(HITRACE_TAG_WINDOW_MANAGER);
    SystemBarRegionTints tints;
    SysBarTintMap& sysBarTintMap = displayGroupController_->sysBarTintMaps_[displayId];
    SysBarNodeMap& sysBarNodeMap = displayGroupController_->sysBarNodeMaps_[displayId];
    for (auto it : sysBarTintMap) { // split screen mode not support yet
        auto sysNode = sysBarNodeMap[it.first];
        if (sysNode == nullptr || it.second.region_ == sysNode->GetWindowRect()) {
            continue;
        }
        const Rect& newRegion = sysNode->GetWindowRect();
        sysBarTintMap[it.first].region_ = newRegion;
        sysBarTintMap[it.first].type_ = it.first;
        tints.emplace_back(sysBarTintMap[it.first]);
        WLOGD("system bar region update, type: %{public}d" \
            "region: [%{public}d, %{public}d, %{public}d, %{public}d]",
            static_cast<int32_t>(it.first), newRegion.posX_, newRegion.posY_, newRegion.width_, newRegion.height_);
    }
    WindowManagerAgentController::GetInstance().UpdateSystemBarRegionTints(displayId, tints);
}

void WindowNodeContainer::NotifyIfKeyboardRegionChanged(const sptr<WindowNode>& node,
    const AvoidControlType avoidType) const
{
    WindowGravity windowGravity;
    uint32_t percent;
    node->GetWindowGravity(windowGravity, percent);
    if (node->GetWindowType() != WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT ||
        windowGravity == WindowGravity::WINDOW_GRAVITY_FLOAT) {
        WLOGFD("windowType: %{public}u", node->GetWindowType());
        return;
    }

    auto callingWindow = FindWindowNodeById(node->GetCallingWindow());
    if (callingWindow == nullptr) {
        WLOGD("callingWindow: %{public}u does not be set", node->GetCallingWindow());
        callingWindow = FindWindowNodeById(GetFocusWindow());
    }
    if (callingWindow == nullptr || callingWindow->GetWindowToken() == nullptr) {
        WLOGE("does not have correct callingWindow for input method window");
        return;
    }
    const WindowMode callingWindowMode = callingWindow->GetWindowMode();
    if (callingWindowMode == WindowMode::WINDOW_MODE_FULLSCREEN ||
        callingWindowMode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
        callingWindowMode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) {
        const Rect keyRect = node->GetWindowRect();
        const Rect callingRect = callingWindow->GetWindowRect();
        if (WindowHelper::IsEmptyRect(WindowHelper::GetOverlap(callingRect, keyRect, 0, 0))) {
            WLOGFD("no overlap between two windows");
            return;
        }
        Rect overlapRect = { 0, 0, 0, 0 };
        if (avoidType == AvoidControlType::AVOID_NODE_ADD || avoidType == AvoidControlType::AVOID_NODE_UPDATE) {
            overlapRect = WindowHelper::GetOverlap(keyRect, callingRect, callingRect.posX_, callingRect.posY_);
        }
        double textFieldPositionY = 0.0;
        double textFieldHeight = 0.0;
        if (node->GetWindowProperty() != nullptr) {
            textFieldPositionY = node->GetWindowProperty()->GetTextFieldPositionY();
            textFieldHeight = node->GetWindowProperty()->GetTextFieldHeight();
        }
        sptr<OccupiedAreaChangeInfo> info = new OccupiedAreaChangeInfo(OccupiedAreaType::TYPE_INPUT,
            overlapRect, textFieldPositionY, textFieldHeight);
        std::map<AvoidAreaType, AvoidArea> avoidAreas = {};
        using T = std::underlying_type_t<AvoidAreaType>;
        for (T type = static_cast<T>(AvoidAreaType::TYPE_START);
            type < static_cast<T>(AvoidAreaType::TYPE_END); type++) {
            auto avoidAreaType = static_cast<AvoidAreaType>(type);
            AvoidArea area = GetAvoidAreaByType(callingWindow, avoidAreaType);
            avoidAreas[avoidAreaType] = area;
        }
        if (callingWindow->GetWindowType() == WindowType::WINDOW_TYPE_DESKTOP ||
            callingWindow->GetWindowType() == WindowType::WINDOW_TYPE_KEYGUARD) {
            avoidAreas = {};
            TLOGD(WmsLogTag::WMS_KEYBOARD, "No need to update immersive avoidarea");
        }
        if (isAnimateTransactionEnabled_) {
            auto rsTransaction = RSSyncTransactionAdapter::GetRSTransaction(node->GetRSUIContext());
            callingWindow->GetWindowToken()->UpdateOccupiedAreaChangeInfo(info, avoidAreas, rsTransaction);
        } else {
            callingWindow->GetWindowToken()->UpdateOccupiedAreaChangeInfo(info, avoidAreas);
        }

        WLOGI("keyboard size change callingWindow: [%{public}s, %{public}u], "
            "overlap rect: [%{public}d, %{public}d, %{public}u, %{public}u]",
            callingWindow->GetWindowName().c_str(), callingWindow->GetWindowId(),
            overlapRect.posX_, overlapRect.posY_, overlapRect.width_, overlapRect.height_);
        return;
    }
    WLOGFE("does not have correct callingWindowMode for input method window");
}

void WindowNodeContainer::NotifySystemBarTints(std::vector<DisplayId> displayIdVec)
{
    if (displayIdVec.size() != displayGroupController_->sysBarTintMaps_.size()) {
        WLOGE("[Immersive] the number of display is error");
    }

    for (auto displayId : displayIdVec) {
        SystemBarRegionTints tints;
        SysBarTintMap& sysBarTintMap = displayGroupController_->sysBarTintMaps_[displayId];
        for (auto it : sysBarTintMap) {
            WLOGI("[Immersive] systembar tints, T: %{public}d, " \
                "V: %{public}d, C: %{public}x | %{public}x, " \
                "R: [%{public}d, %{public}d, %{public}d, %{public}d]",
                static_cast<int32_t>(it.first),
                sysBarTintMap[it.first].prop_.enable_,
                sysBarTintMap[it.first].prop_.backgroundColor_, sysBarTintMap[it.first].prop_.contentColor_,
                sysBarTintMap[it.first].region_.posX_, sysBarTintMap[it.first].region_.posY_,
                sysBarTintMap[it.first].region_.width_, sysBarTintMap[it.first].region_.height_);
            tints.push_back(sysBarTintMap[it.first]);
        }
        WindowManagerAgentController::GetInstance().UpdateSystemBarRegionTints(displayId, tints);
    }
}

void WindowNodeContainer::NotifyDockWindowStateChanged(sptr<WindowNode>& node, bool isEnable)
{
    HITRACE_METER(HITRACE_TAG_WINDOW_MANAGER);
    WLOGFD("[Immersive] begin isEnable: %{public}d", isEnable);
    if (isEnable) {
        for (auto& windowNode : appWindowNode_->children_) {
            if (windowNode->GetWindowId() == node->GetWindowId()) {
                continue;
            }
            if (!WindowHelper::IsFloatingWindow(windowNode->GetWindowMode())) {
                return;
            }
        }
    }
    SystemBarProperty prop;
    prop.enable_ = isEnable;
    SystemBarRegionTint tint;
    tint.type_ = WindowType::WINDOW_TYPE_LAUNCHER_DOCK;
    tint.prop_ = prop;
    SystemBarRegionTints tints;
    tints.push_back(tint);
    WindowManagerAgentController::GetInstance().UpdateSystemBarRegionTints(node->GetDisplayId(), tints);
}

void WindowNodeContainer::NotifyDockWindowStateChanged(DisplayId displayId)
{
    HITRACE_METER(HITRACE_TAG_WINDOW_MANAGER);
    bool isEnable = true;
    for (auto& windowNode : appWindowNode_->children_) {
        if (WindowHelper::IsSplitWindowMode(windowNode->GetWindowMode()) ||
            WindowHelper::IsFullScreenWindow(windowNode->GetWindowMode())) {
            isEnable = false;
            break;
        }
    }
    WLOGFD("[Immersive] display %{public}" PRIu64" begin isEnable: %{public}d", displayId, isEnable);
    SystemBarProperty prop;
    prop.enable_ = isEnable;
    SystemBarRegionTint tint;
    tint.type_ = WindowType::WINDOW_TYPE_LAUNCHER_DOCK;
    tint.prop_ = prop;
    SystemBarRegionTints tints;
    tints.push_back(tint);
    WindowManagerAgentController::GetInstance().UpdateSystemBarRegionTints(displayId, tints);
}

void WindowNodeContainer::UpdateAvoidAreaListener(sptr<WindowNode>& windowNode, bool haveAvoidAreaListener)
{
    avoidController_->UpdateAvoidAreaListener(windowNode, haveAvoidAreaListener);
}

bool WindowNodeContainer::IsTopWindow(uint32_t windowId, sptr<WindowNode>& rootNode) const
{
    if (rootNode->children_.empty()) {
        WLOGFE("root does not have any node");
        return false;
    }
    auto node = *(rootNode->children_.rbegin());
    if (node == nullptr) {
        WLOGFE("window tree does not have any node");
        return false;
    }

    for (auto iter = node->children_.rbegin(); iter < node->children_.rend(); iter++) {
        if ((*iter)->priority_ > 0) {
            return (*iter)->GetWindowId() == windowId;
        } else {
            break;
        }
    }
    return node->GetWindowId() == windowId;
}

void WindowNodeContainer::RaiseOrderedWindowToTop(std::vector<sptr<WindowNode>>& orderedNodes,
    std::vector<sptr<WindowNode>>& windowNodes)
{
    for (auto iter = appWindowNode_->children_.begin(); iter != appWindowNode_->children_.end();) {
        uint32_t wid = (*iter)->GetWindowId();
        auto orderedIter = std::find_if(orderedNodes.begin(), orderedNodes.end(),
            [wid] (sptr<WindowNode> orderedNode) { return orderedNode->GetWindowId() == wid; });
        if (orderedIter != orderedNodes.end()) {
            iter = windowNodes.erase(iter);
        } else {
            iter++;
        }
    }
    for (auto iter = orderedNodes.begin(); iter != orderedNodes.end(); iter++) {
        UpdateWindowTree(*iter);
    }
    return;
}

void WindowNodeContainer::RaiseWindowToTop(uint32_t windowId, std::vector<sptr<WindowNode>>& windowNodes)
{
    if (windowNodes.empty()) {
        WLOGFE("windowNodes is empty!");
        return;
    }
    auto iter = std::find_if(windowNodes.begin(), windowNodes.end(),
                             [windowId](sptr<WindowNode> node) {
                                 return node->GetWindowId() == windowId;
                             });
    // raise app node window to top
    if (iter != windowNodes.end()) {
        sptr<WindowNode> node = *iter;
        windowNodes.erase(iter);
        UpdateWindowTree(node);
        WLOGD("raise window to top %{public}u", node->GetWindowId());
    }
}

void WindowNodeContainer::TraverseContainer(std::vector<sptr<WindowNode>>& windowNodes) const
{
    for (auto& node : belowAppWindowNode_->children_) {
        TraverseWindowNode(node, windowNodes);
    }
    for (auto& node : appWindowNode_->children_) {
        TraverseWindowNode(node, windowNodes);
    }
    for (auto& node : aboveAppWindowNode_->children_) {
        TraverseWindowNode(node, windowNodes);
    }
    std::reverse(windowNodes.begin(), windowNodes.end());
}

void WindowNodeContainer::TraverseWindowNode(sptr<WindowNode>& node, std::vector<sptr<WindowNode>>& windowNodes) const
{
    if (node == nullptr) {
        return;
    }
    auto iter = node->children_.begin();
    for (; iter < node->children_.end(); ++iter) {
        if ((*iter)->priority_ < 0) {
            windowNodes.emplace_back(*iter);
        } else {
            break;
        }
    }
    windowNodes.emplace_back(node);
    for (; iter < node->children_.end(); ++iter) {
        windowNodes.emplace_back(*iter);
    }
}

AvoidArea WindowNodeContainer::GetAvoidAreaByType(const sptr<WindowNode>& node, AvoidAreaType avoidAreaType,
    const Rect& rect) const
{
    if (CheckWindowNodeWhetherInWindowTree(node)) {
        return avoidController_->GetAvoidAreaByType(node, avoidAreaType);
    }
    return {};
}

bool WindowNodeContainer::CheckWindowNodeWhetherInWindowTree(const sptr<WindowNode>& node) const
{
    bool isInWindowTree = false;
    WindowNodeOperationFunc func = [&node, &isInWindowTree](sptr<WindowNode> windowNode) {
        if (node->GetWindowId() == windowNode->GetWindowId()) {
            isInWindowTree = true;
            return true;
        }
        return false;
    };
    TraverseWindowTree(func, true);
    return isInWindowTree;
}

void WindowNodeContainer::DumpScreenWindowTreeByWinId(uint32_t winid)
{
    WLOGFD("------ dump window info begin -------");
    WLOGFD("WindowName WinId Type Mode ZOrd [   x    y    w    h]");
    uint32_t zOrder = zOrder_;
    WindowNodeOperationFunc func = [&zOrder, &winid](sptr<WindowNode> node) {
        Rect rect = node->GetWindowRect();
        uint32_t windowId = node->GetWindowId();
        const std::string& windowName = node->GetWindowName().size() < WINDOW_NAME_MAX_LENGTH ?
            node->GetWindowName() : node->GetWindowName().substr(0, WINDOW_NAME_MAX_LENGTH);
        if (winid == windowId) {
            WLOGD("DumpScreenWindowTree: %{public}10s  %{public}5u %{public}4u %{public}4u "
                "%{public}4u [%{public}4d %{public}4d %{public}4u %{public}4u]",
                windowName.c_str(), node->GetWindowId(), node->GetWindowType(), node->GetWindowMode(),
                --zOrder, rect.posX_, rect.posY_, rect.width_, rect.height_);
        } else {
            WLOGD("DumpScreenWindowTree: %{public}10s  %{public}5u %{public}4u %{public}4u "
                "%{public}4u [%{public}4d %{public}4d %{public}4u %{public}4u]",
                windowName.c_str(), node->GetWindowId(), node->GetWindowType(), node->GetWindowMode(),
                --zOrder, rect.posX_, rect.posY_, rect.width_, rect.height_);
        }
        return false;
    };
    TraverseWindowTree(func, true);
    WLOGFD("------ dump window info end -------");
}

void WindowNodeContainer::DumpScreenWindowTree()
{
    WLOGI("------ dump window info begin -------");
    WLOGI("WindowName DisplayId WinId Type Mode Flag ZOrd Orientation firstFrameCallback [   x    y    w    h]");
    uint32_t zOrder = zOrder_;
    WindowNodeOperationFunc func = [&zOrder](sptr<WindowNode> node) {
        Rect rect = node->GetWindowRect();
        const std::string& windowName = node->GetWindowName().size() < WINDOW_NAME_MAX_LENGTH ?
            node->GetWindowName() : node->GetWindowName().substr(0, WINDOW_NAME_MAX_LENGTH);
        WLOGI("DumpScreenWindowTree: %{public}10s %{public}9" PRIu64" %{public}5u %{public}4u %{public}4u %{public}4u "
            "%{public}4u %{public}11u %{public}12d [%{public}4d %{public}4d %{public}4u %{public}4u]",
            windowName.c_str(), node->GetDisplayId(), node->GetWindowId(), node->GetWindowType(), node->GetWindowMode(),
            node->GetWindowFlags(), --zOrder, static_cast<uint32_t>(node->GetRequestedOrientation()),
            node->firstFrameAvailable_, rect.posX_, rect.posY_, rect.width_, rect.height_);
        return false;
    };
    TraverseWindowTree(func, true);
    WLOGI("------ dump window info end -------");
}

bool WindowNodeContainer::IsVerticalDisplay(DisplayId displayId) const
{
    return DisplayGroupInfo::GetInstance().GetDisplayRect(displayId).width_ <
        DisplayGroupInfo::GetInstance().GetDisplayRect(displayId).height_;
}

void WindowNodeContainer::ProcessWindowStateChange(WindowState state, WindowStateChangeReason reason)
{
    switch (reason) {
        case WindowStateChangeReason::KEYGUARD: {
            int32_t topPriority = zorderPolicy_->GetWindowPriority(WindowType::WINDOW_TYPE_KEYGUARD);
            TraverseAndUpdateWindowState(state, topPriority);
            break;
        }
        default:
            return;
    }
}

void WindowNodeContainer::TraverseAndUpdateWindowState(WindowState state, int32_t topPriority)
{
    std::vector<sptr<WindowNode>> rootNodes = { belowAppWindowNode_, appWindowNode_, aboveAppWindowNode_ };
    for (auto& node : rootNodes) {
        UpdateWindowState(node, topPriority, state);
    }
}

void WindowNodeContainer::UpdateWindowState(sptr<WindowNode> node, int32_t topPriority, WindowState state)
{
    if (node == nullptr) {
        return;
    }
    if (node->parent_ != nullptr && node->currentVisibility_) {
        if (node->priority_ < topPriority && !WindowHelper::IsShowWhenLocked(node->GetWindowFlags()) &&
            !WindowHelper::IsShowWhenLocked(node->parent_->GetWindowFlags())) {
            if (node->GetWindowToken()) {
                node->GetWindowToken()->UpdateWindowState(state);
            }
            HandleKeepScreenOn(node, state);
        }
    }
    for (auto& childNode : node->children_) {
        UpdateWindowState(childNode, topPriority, state);
    }
}

void WindowNodeContainer::HandleKeepScreenOn(const sptr<WindowNode>& node, WindowState state)
{
    if (node == nullptr) {
        WLOGFE("window is invalid");
        return;
    }
    if (state == WindowState::STATE_FROZEN) {
        HandleKeepScreenOn(node, false);
    } else if (state == WindowState::STATE_UNFROZEN) {
        HandleKeepScreenOn(node, node->IsKeepScreenOn());
    } else {
        // do nothing
    }
}

sptr<WindowNode> WindowNodeContainer::FindDividerNode() const
{
    for (auto iter = appWindowNode_->children_.begin(); iter != appWindowNode_->children_.end(); iter++) {
        if ((*iter)->GetWindowType() == WindowType::WINDOW_TYPE_DOCK_SLICE) {
            return *iter;
        }
    }
    return nullptr;
}

void WindowNodeContainer::RaiseSplitRelatedWindowToTop(sptr<WindowNode>& node)
{
    if (node == nullptr) {
        return;
    }
    auto windowPair = displayGroupController_->GetWindowPairByDisplayId(node->GetDisplayId());
    if (windowPair == nullptr) {
        WLOGFE("Window pair is nullptr");
        return;
    }
    std::vector<sptr<WindowNode>> orderedPair = windowPair->GetOrderedPair(node);
    RaiseOrderedWindowToTop(orderedPair, appWindowNode_->children_);
    AssignZOrder();
    return;
}

WMError WindowNodeContainer::RaiseZOrderForAppWindow(sptr<WindowNode>& node, sptr<WindowNode>& parentNode)
{
    if (node == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }
    if (IsTopWindow(node->GetWindowId(), appWindowNode_) || IsTopWindow(node->GetWindowId(), aboveAppWindowNode_)) {
        WLOGE("Window %{public}u is already at top", node->GetWindowId());
        return WMError::WM_ERROR_INVALID_TYPE;
    }

    if (WindowHelper::IsSubWindow(node->GetWindowType()) ||
        (node->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG)) {
        if (parentNode == nullptr) {
            WLOGFE("window type is invalid");
            return WMError::WM_ERROR_NULLPTR;
        }
        RaiseWindowToTop(node->GetWindowId(), parentNode->children_); // raise itself
        if (parentNode->IsSplitMode()) {
            RaiseSplitRelatedWindowToTop(parentNode);
        } else if (parentNode->parent_ != nullptr) {
            RaiseWindowToTop(parentNode->GetWindowId(), parentNode->parent_->children_); // raise parent window
        }
    } else if (WindowHelper::IsMainWindow(node->GetWindowType())) {
        if (node->IsSplitMode()) {
            RaiseSplitRelatedWindowToTop(node);
        } else {
            // remote animation continuous start and exit allow parent is nullptr
            if (node->parent_ == nullptr) {
                WLOGFW("node parent is nullptr");
                return WMError::WM_OK;
            }
            RaiseWindowToTop(node->GetWindowId(), node->parent_->children_);
        }
    } else {
        // do nothing
    }

    AssignZOrder();
    WLOGI("Raise app window zorder");
    DumpScreenWindowTreeByWinId(node->GetWindowId());
    return WMError::WM_OK;
}

sptr<WindowNode> WindowNodeContainer::GetNextFocusableWindow(uint32_t windowId) const
{
    sptr<WindowNode> nextFocusableWindow;
    bool previousFocusedWindowFound = false;
    WindowNodeOperationFunc func = [windowId, &nextFocusableWindow, &previousFocusedWindowFound](
        sptr<WindowNode> node) {
        if (previousFocusedWindowFound && node->GetWindowProperty()->GetFocusable() && node->currentVisibility_) {
            nextFocusableWindow = node;
            return true;
        }
        if (node->GetWindowId() == windowId) {
            previousFocusedWindowFound = true;
        }
        return false;
    };
    TraverseWindowTree(func, true);
    return nextFocusableWindow;
}

sptr<WindowNode> WindowNodeContainer::GetNextRotatableWindow(uint32_t windowId) const
{
    sptr<WindowNode> nextRotatableWindow;
    WindowNodeOperationFunc func = [windowId, &nextRotatableWindow](
        sptr<WindowNode> node) {
        if (windowId != node->GetWindowId() &&
            WindowHelper::IsRotatableWindow(node->GetWindowType(), node->GetWindowMode())) {
            nextRotatableWindow = node;
            return true;
        }
        return false;
    };
    TraverseWindowTree(func, true);
    return nextRotatableWindow;
}

sptr<WindowNode> WindowNodeContainer::GetNextActiveWindow(uint32_t windowId) const
{
    auto currentNode = FindWindowNodeById(windowId);
    if (currentNode == nullptr) {
        WLOGFE("cannot find window id: %{public}u by tree", windowId);
        return nullptr;
    }
    WLOGFD("current window: [%{public}u, %{public}u]", windowId, static_cast<uint32_t>(currentNode->GetWindowType()));
    if (WindowHelper::IsSystemWindow(currentNode->GetWindowType())) {
        for (auto& node : appWindowNode_->children_) {
            if (node->GetWindowType() == WindowType::WINDOW_TYPE_DOCK_SLICE) {
                continue;
            }
            return node;
        }
        for (auto& node : belowAppWindowNode_->children_) {
            if (node->GetWindowType() == WindowType::WINDOW_TYPE_DESKTOP) {
                return node;
            }
        }
    } else if (WindowHelper::IsAppWindow(currentNode->GetWindowType())) {
        std::vector<sptr<WindowNode>> windowNodes;
        TraverseContainer(windowNodes);
        auto iter = std::find_if(windowNodes.begin(), windowNodes.end(), [windowId](sptr<WindowNode>& node) {
            return node->GetWindowId() == windowId;
            });
        if (iter == windowNodes.end()) {
            WLOGFE("could not find this window");
            return nullptr;
        }
        int index = std::distance(windowNodes.begin(), iter);
        for (size_t i = static_cast<size_t>(index) + 1; i < windowNodes.size(); i++) {
            if (windowNodes[i]->GetWindowType() == WindowType::WINDOW_TYPE_DOCK_SLICE
                || !windowNodes[i]->currentVisibility_) {
                continue;
            }
            return windowNodes[i];
        }
    } else {
        // do nothing
    }
    WLOGFE("could not get next active window");
    return nullptr;
}

bool WindowNodeContainer::IsForbidDockSliceMove(DisplayId displayId) const
{
    auto windowPair = displayGroupController_->GetWindowPairByDisplayId(displayId);
    if (windowPair == nullptr) {
        WLOGFE("window pair is nullptr");
        return true;
    }
    if (windowPair->IsForbidDockSliceMove()) {
        return true;
    }
    return false;
}

bool WindowNodeContainer::IsDockSliceInExitSplitModeArea(DisplayId displayId) const
{
    auto windowPair = displayGroupController_->GetWindowPairByDisplayId(displayId);
    if (windowPair == nullptr) {
        WLOGFE("window pair is nullptr");
        return false;
    }
    std::vector<int32_t> exitSplitPoints = windowPair->GetExitSplitPoints();
    if (exitSplitPoints.size() != EXIT_SPLIT_POINTS_NUMBER) {
        return false;
    }
    return windowPair->IsDockSliceInExitSplitModeArea(exitSplitPoints);
}

void WindowNodeContainer::ExitSplitMode(DisplayId displayId)
{
    auto windowPair = displayGroupController_->GetWindowPairByDisplayId(displayId);
    if (windowPair == nullptr) {
        WLOGFE("window pair is nullptr");
        return;
    }
    windowPair->ExitSplitMode();
}

void WindowNodeContainer::MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId)
{
    WMError ret = MinimizeAppNodeExceptOptions(MinimizeReason::MINIMIZE_ALL);
    SwitchLayoutPolicy(WindowLayoutMode::CASCADE, displayId);
    if (ret != WMError::WM_OK) {
        WLOGFE("Minimize all app window failed");
    }
    return;
}

sptr<WindowNode> WindowNodeContainer::GetDeskTopWindow()
{
    sptr<WindowNode> deskTop;
    WindowNodeOperationFunc findDeskTopFunc = [this, &deskTop](sptr<WindowNode> node) {
        if (node->GetWindowProperty()->GetWindowType() == WindowType::WINDOW_TYPE_DESKTOP) {
            deskTop = node;
            return true;
        }
        return false;
    };
    TraverseWindowTree(findDeskTopFunc, false);
    return deskTop;
}

bool WindowNodeContainer::HasPrivateWindow()
{
    std::vector<sptr<WindowNode>> windowNodes;
    TraverseContainer(windowNodes);
    for (const auto& node : windowNodes) {
        if (node && node->GetVisibilityState() < WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION &&
            node->GetWindowProperty()->GetPrivacyMode()) {
            WLOGI("window name %{public}s", node->GetWindowName().c_str());
            return true;
        }
    }
    return false;
}

bool WindowNodeContainer::HasMainFullScreenWindowShown()
{
    std::vector<sptr<WindowNode>> windowNodes;
    for (auto& node : appWindowNode_->children_) {
        TraverseWindowNode(node, windowNodes);
    }
    for (const auto& node : windowNodes) {
        if (node->currentVisibility_ &&
            WindowHelper::IsMainFullScreenWindow(node->GetWindowType(), node->GetWindowMode())) {
            return true;
        }
    }
    return false;
}

void WindowNodeContainer::MinimizeOldestAppWindow()
{
    for (auto& appNode : appWindowNode_->children_) {
        if (appNode->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
            MinimizeApp::AddNeedMinimizeApp(appNode, MinimizeReason::MAX_APP_COUNT);
            return;
        }
    }
    for (auto& appNode : aboveAppWindowNode_->children_) {
        if (appNode->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
            MinimizeApp::AddNeedMinimizeApp(appNode, MinimizeReason::MAX_APP_COUNT);
            return;
        }
    }
    WLOGD("no window needs to minimize");
}

void WindowNodeContainer::MinimizeOldestMainFloatingWindow(uint32_t windowId)
{
    if (maxMainFloatingWindowNumber_ <= 0) {
        WLOGD("There is no limit at The number of floating window");
        return;
    }

    auto windowNumber = GetMainFloatingWindowCount();
    if (windowNumber <= maxMainFloatingWindowNumber_) {
        WLOGD("The number of floating window is less then MaxFloatAppMainWindowNumber");
        return;
    }
    std::vector<sptr<WindowNode>> rootNodes = {
        appWindowNode_, aboveAppWindowNode_,
    };
    for (auto& root : rootNodes) {
        for (auto& appNode : root->children_) {
            WindowType windowType = appNode->GetWindowType();
            WindowMode windowMode = appNode->GetWindowMode();
            uint32_t winId = appNode->GetWindowId();
            if (windowId != winId && WindowHelper::IsMainFloatingWindow(windowType, windowMode)) {
                MinimizeApp::AddNeedMinimizeApp(appNode, MinimizeReason::MAX_APP_COUNT);
                return;
            }
        }
    }
    WLOGD("no window needs to minimize");
}

WMError WindowNodeContainer::ToggleShownStateForAllAppWindows(
    std::function<bool(uint32_t, WindowMode)> restoreFunc, bool restore)
{
    for (auto node : aboveAppWindowNode_->children_) {
        if (node->GetWindowType() == WindowType::WINDOW_TYPE_LAUNCHER_RECENT &&
            node->GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN && restore) {
            return WMError::WM_DO_NOTHING;
        }
    }
    // to do, backup reentry: 1.ToggleShownStateForAllAppWindows fast; 2.this display should reset backupWindowIds_.
    if (!restore && appWindowNode_->children_.empty() && !backupWindowIds_.empty()) {
        backupWindowIds_.clear();
        backupWindowMode_.clear();
        backupDisplaySplitWindowMode_.clear();
        backupDividerWindowRect_.clear();
    }
    if (!restore && !appWindowNode_->children_.empty() && backupWindowIds_.empty()) {
        WLOGD("backup");
        BackUpAllAppWindows();
    } else if (restore && !backupWindowIds_.empty()) {
        WLOGD("restore");
        RestoreAllAppWindows(restoreFunc);
    } else {
        WLOGD("do nothing because shown app windows is empty or backup windows is empty.");
    }
    WLOGD("ToggleShownStateForAllAppWindows");
    return WMError::WM_OK;
}

void WindowNodeContainer::BackUpAllAppWindows()
{
    std::set<DisplayId> displayIdSet;
    backupWindowMode_.clear();
    backupDisplaySplitWindowMode_.clear();
    std::vector<sptr<WindowNode>> children = appWindowNode_->children_;
    for (auto& appNode : children) {
        if (!WindowHelper::IsMainWindow(appNode->GetWindowType())) {
            continue;
        }
        auto windowMode = appNode->GetWindowMode();
        backupWindowMode_[appNode->GetWindowId()] = windowMode;
        if (WindowHelper::IsSplitWindowMode(windowMode)) {
            backupDisplaySplitWindowMode_[appNode->GetDisplayId()].insert(windowMode);
        }
        displayIdSet.insert(appNode->GetDisplayId());
    }
    for (auto& appNode : children) {
        // exclude exceptional window
        if (!WindowHelper::IsMainWindow(appNode->GetWindowType())) {
            WLOGFE("is not main window, windowId:%{public}u", appNode->GetWindowId());
            continue;
        }
        // minimize window
        WLOGFD("minimize window, windowId:%{public}u", appNode->GetWindowId());
        backupWindowIds_.emplace_back(appNode->GetWindowId());
        WindowManagerService::GetInstance().RemoveWindow(appNode->GetWindowId(), true);
        wptr<IRemoteObject> abilityToken = appNode->abilityToken_;
        auto task = [abilityToken]() {
            auto token = abilityToken.promote();
            if (token == nullptr) {
                WLOGFW("Ability token is null");
                return;
            }
            AAFwk::AbilityManagerClient::GetInstance()->DoAbilityBackground(token,
                static_cast<uint32_t>(WindowStateChangeReason::TOGGLING));
        };
        WindowInnerManager::GetInstance().PostTask(task, "DoAbilityBackground");
    }
    backupDividerWindowRect_.clear();
    for (auto displayId : displayIdSet) {
        auto windowPair = displayGroupController_->GetWindowPairByDisplayId(displayId);
        if (windowPair == nullptr || windowPair->GetDividerWindow() == nullptr) {
            continue;
        }
        backupDividerWindowRect_[displayId] = windowPair->GetDividerWindow()->GetWindowRect();
    }
}

void WindowNodeContainer::RestoreAllAppWindows(std::function<bool(uint32_t, WindowMode)> restoreFunc)
{
    std::vector<uint32_t> backupWindowIds(backupWindowIds_);
    auto displayIds = DisplayGroupInfo::GetInstance().GetAllDisplayIds();
    std::vector<sptr<WindowPair>> windowPairs;
    for (auto displayId : displayIds) {
        auto windowPair = displayGroupController_->GetWindowPairByDisplayId(displayId);
        if (windowPair != nullptr) {
            if (backupDisplaySplitWindowMode_[displayId].count(WindowMode::WINDOW_MODE_SPLIT_PRIMARY) > 0 &&
                backupDisplaySplitWindowMode_[displayId].count(WindowMode::WINDOW_MODE_SPLIT_SECONDARY) > 0) {
                windowPair->SetAllSplitAppWindowsRestoring(true);
            }
            windowPairs.emplace_back(windowPair);
        }
    }
    for (auto windowId: backupWindowIds) {
        if (!restoreFunc(windowId, backupWindowMode_[windowId])) {
            WLOGFE("restore %{public}u failed", windowId);
            continue;
        }
        WLOGFD("restore %{public}u", windowId);
    }
    for (auto windowPair : windowPairs) {
        windowPair->SetAllSplitAppWindowsRestoring(false);
    }
    layoutPolicy_->SetSplitDividerWindowRects(backupDividerWindowRect_);
    backupWindowIds_.clear();
    backupWindowMode_.clear();
    backupDividerWindowRect_.clear();
}

bool WindowNodeContainer::IsAppWindowsEmpty() const
{
    return appWindowNode_->children_.empty();
}

WMError WindowNodeContainer::MinimizeAppNodeExceptOptions(MinimizeReason reason,
    const std::vector<uint32_t>& exceptionalIds, const std::vector<WindowMode>& exceptionalModes)
{
    if (appWindowNode_->children_.empty()) {
        return WMError::WM_OK;
    }
    for (auto& appNode : appWindowNode_->children_) {
        // exclude exceptional window
        if (std::find(exceptionalIds.begin(), exceptionalIds.end(), appNode->GetWindowId()) != exceptionalIds.end() ||
            std::find(exceptionalModes.begin(), exceptionalModes.end(),
                appNode->GetWindowMode()) != exceptionalModes.end() ||
                appNode->GetWindowType() != WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
            continue;
        }
        MinimizeApp::AddNeedMinimizeApp(appNode, reason);
    }
    return WMError::WM_OK;
}

WMError WindowNodeContainer::MinimizeStructuredAppWindowsExceptSelf(const sptr<WindowNode>& node)
{
    std::vector<uint32_t> exceptionalIds = { node->GetWindowId() };
    std::vector<WindowMode> exceptionalModes = { WindowMode::WINDOW_MODE_FLOATING, WindowMode::WINDOW_MODE_PIP };
    return MinimizeAppNodeExceptOptions(MinimizeReason::OTHER_WINDOW, exceptionalIds, exceptionalModes);
}

WMError WindowNodeContainer::SwitchLayoutPolicy(WindowLayoutMode dstMode, DisplayId displayId, bool reorder)
{
    WLOGD("SwitchLayoutPolicy src: %{public}d dst: %{public}d, reorder: %{public}d, displayId: %{public}" PRIu64"",
        static_cast<uint32_t>(layoutMode_), static_cast<uint32_t>(dstMode), static_cast<uint32_t>(reorder), displayId);
    if (dstMode < WindowLayoutMode::BASE || dstMode >= WindowLayoutMode::END) {
        WLOGFE("invalid layout mode");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    auto windowPair = displayGroupController_->GetWindowPairByDisplayId(displayId);
    if (windowPair == nullptr) {
        WLOGFE("Window pair is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (layoutMode_ != dstMode) {
        if (layoutMode_ == WindowLayoutMode::CASCADE) {
            windowPair->Clear();
        }
        layoutMode_ = dstMode;
        layoutPolicy_ = layoutPolicies_[dstMode];
        layoutPolicy_->Launch();
        DumpScreenWindowTree();
    } else {
        WLOGI("Current layout mode is already: %{public}d", static_cast<uint32_t>(dstMode));
    }
    if (reorder) {
        windowPair->Clear();
        layoutPolicy_->Reorder();
        DumpScreenWindowTree();
    }
    NotifyIfSystemBarTintChanged(displayId);
    NotifyDockWindowStateChanged(displayId);
    return WMError::WM_OK;
}

void WindowNodeContainer::UpdateWindowModeSupportTypeWhenKeyguardChange(const sptr<WindowNode>& node, bool up)
{
    if (!WindowHelper::IsWindowModeSupported(node->GetWindowProperty()->GetRequestWindowModeSupportType(),
                                             WindowMode::WINDOW_MODE_SPLIT_PRIMARY)) {
        WLOGFD("window doesn't support split mode, winId: %{public}d", node->GetWindowId());
        return;
    }
    uint32_t windowModeSupportType;
    if (up) {
        windowModeSupportType = node->GetWindowModeSupportType() &
            (~WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY);
    } else {
        windowModeSupportType = node->GetWindowModeSupportType() | WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY;
    }
    node->SetWindowModeSupportType(windowModeSupportType);
    if (node->GetWindowToken() != nullptr) {
        node->GetWindowToken()->UpdateWindowModeSupportType(windowModeSupportType);
    }
}

void WindowNodeContainer::RaiseInputMethodWindowPriorityIfNeeded(const sptr<WindowNode>& node) const
{
    if (node->GetWindowType() != WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        return;
    }

    if (isScreenLocked_) {
        node->priority_ = zorderPolicy_->GetWindowPriority(
            WindowType::WINDOW_TYPE_KEYGUARD) + 2; // 2: higher than keyguard and show when locked window
        WLOGD("Raise input method float window priority when screen locked.");
        return;
    }

    auto callingWindowId = node->GetCallingWindow();
    auto callingWindow = FindWindowNodeById(callingWindowId);
    if (callingWindowId == focusedWindow_ && callingWindow != nullptr) {
        auto callingWindowType = callingWindow->GetWindowType();
        auto callingWindowPriority = zorderPolicy_->GetWindowPriority(callingWindowType);
        auto inputMethodPriority = zorderPolicy_->GetWindowPriority(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);

        node->priority_ = (inputMethodPriority < callingWindowPriority) ?
            (callingWindowPriority + 1) : inputMethodPriority;
        WLOGFD("Reset input method float window priority to %{public}d.", node->priority_);
        return;
    }

    auto focusWindow = FindWindowNodeById(focusedWindow_);
    if (focusWindow != nullptr && focusWindow->GetWindowType() == WindowType::WINDOW_TYPE_PANEL) {
        node->priority_ = zorderPolicy_->GetWindowPriority(WindowType::WINDOW_TYPE_PANEL) + 1;
        WLOGFD("The input method float window should be higher than panel");
    }
}

void WindowNodeContainer::ReZOrderShowWhenLockedWindows(bool up)
{
    WLOGD("Keyguard change %{public}u, re-zorder showWhenLocked window", up);
    std::vector<sptr<WindowNode>> needReZOrderNodes;
    auto& srcRoot = up ? appWindowNode_ : aboveAppWindowNode_;
    auto& dstRoot = up ? aboveAppWindowNode_ : appWindowNode_;

    auto dstPriority = up ? zorderPolicy_->GetWindowPriority(WindowType::WINDOW_TYPE_KEYGUARD) + 1 :
        zorderPolicy_->GetWindowPriority(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    for (auto iter = srcRoot->children_.begin(); iter != srcRoot->children_.end();) {
        if ((*iter)->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED)) {
            needReZOrderNodes.emplace_back(*iter);
            iter = srcRoot->children_.erase(iter);
        } else {
            iter++;
        }
    }
    const int32_t floatingPriorityOffset = 1;
    for (auto& needReZOrderNode : needReZOrderNodes) {
        needReZOrderNode->priority_ = dstPriority;
        needReZOrderNode->parent_ = dstRoot;
        if (WindowHelper::IsMainFloatingWindow(needReZOrderNode->GetWindowType(),
            needReZOrderNode->GetWindowMode()) && isFloatWindowAboveFullWindow_) {
            needReZOrderNode->priority_ = dstPriority + floatingPriorityOffset;
        }
        auto parentNode = needReZOrderNode->parent_;
        auto position = parentNode->children_.end();
        for (auto iter = parentNode->children_.begin(); iter < parentNode->children_.end(); ++iter) {
            if ((*iter)->priority_ > needReZOrderNode->priority_) {
                position = iter;
                break;
            }
        }

        UpdateWindowModeSupportTypeWhenKeyguardChange(needReZOrderNode, up);

        parentNode->children_.insert(position, needReZOrderNode);
        if (up && WindowHelper::IsSplitWindowMode(needReZOrderNode->GetWindowMode())) {
            needReZOrderNode->GetWindowProperty()->ResumeLastWindowMode();
            // when change mode, need to reset shadow and radius
            WindowSystemEffect::SetWindowEffect(needReZOrderNode);
            if (needReZOrderNode->GetWindowToken() != nullptr) {
                needReZOrderNode->GetWindowToken()->UpdateWindowMode(needReZOrderNode->GetWindowMode());
            }
            auto windowPair = displayGroupController_->GetWindowPairByDisplayId(needReZOrderNode->GetDisplayId());
            if (windowPair == nullptr) {
                WLOGFE("Window pair is nullptr");
                return;
            }
            windowPair->UpdateIfSplitRelated(needReZOrderNode);
        }
        WLOGD("window %{public}u re-zorder when keyguard change %{public}u", needReZOrderNode->GetWindowId(), up);
    }
}

void WindowNodeContainer::ReZOrderShowWhenLockedWindowIfNeeded(const sptr<WindowNode>& node)
{
    if (!(node->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED)) ||
        !isScreenLocked_) {
        return;
    }

    ReZOrderShowWhenLockedWindows(true);
    WLOGI("ShowWhenLocked window %{public}u re-zorder to up", node->GetWindowId());
}

void WindowNodeContainer::RaiseShowWhenLockedWindowIfNeeded(const sptr<WindowNode>& node)
{
    // if keyguard window show, raise show when locked windows
    if (node->GetWindowType() == WindowType::WINDOW_TYPE_KEYGUARD) {
        ReZOrderShowWhenLockedWindows(true);
        return;
    }

    // if show when locked window show, raise itself when exist keyguard
    if (!(node->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED)) ||
        !isScreenLocked_) {
        return;
    }

    node->priority_ = zorderPolicy_->GetWindowPriority(WindowType::WINDOW_TYPE_KEYGUARD) + 1;
    node->parent_ = aboveAppWindowNode_;
    if (WindowHelper::IsSplitWindowMode(node->GetWindowMode())) {
        node->GetWindowProperty()->ResumeLastWindowMode();
        // when change mode, need to reset shadow and radius
        WindowSystemEffect::SetWindowEffect(node);
        if (node->GetWindowToken() != nullptr) {
            node->GetWindowToken()->UpdateWindowMode(node->GetWindowMode());
        }
    }
    WLOGI("ShowWhenLocked window %{public}u raise itself", node->GetWindowId());
}

void WindowNodeContainer::DropShowWhenLockedWindowIfNeeded(const sptr<WindowNode>& node)
{
    // if keyguard window hide, drop show when locked windows
    if (node->GetWindowType() == WindowType::WINDOW_TYPE_KEYGUARD) {
        ReZOrderShowWhenLockedWindows(false);
        AssignZOrder();
    }
}

void WindowNodeContainer::TraverseWindowTree(const WindowNodeOperationFunc& func, bool isFromTopToBottom) const
{
    std::vector<sptr<WindowNode>> rootNodes = { belowAppWindowNode_, appWindowNode_, aboveAppWindowNode_ };
    if (isFromTopToBottom) {
        std::reverse(rootNodes.begin(), rootNodes.end());
    }

    for (const auto& node : rootNodes) {
        if (isFromTopToBottom) {
            for (auto iter = node->children_.rbegin(); iter != node->children_.rend(); ++iter) {
                if (TraverseFromTopToBottom(*iter, func)) {
                    return;
                }
            }
        } else {
            for (auto iter = node->children_.begin(); iter != node->children_.end(); ++iter) {
                if (TraverseFromBottomToTop(*iter, func)) {
                    return;
                }
            }
        }
    }
}

bool WindowNodeContainer::TraverseFromTopToBottom(sptr<WindowNode> node, const WindowNodeOperationFunc& func) const
{
    if (node == nullptr) {
        return false;
    }
    auto iterBegin = node->children_.rbegin();
    for (; iterBegin != node->children_.rend(); ++iterBegin) {
        if ((*iterBegin)->priority_ <= 0) {
            break;
        }
        if (func(*iterBegin)) {
            return true;
        }
    }
    if (func(node)) {
        return true;
    }
    for (; iterBegin != node->children_.rend(); ++iterBegin) {
        if (func(*iterBegin)) {
            return true;
        }
    }
    return false;
}

bool WindowNodeContainer::TraverseFromBottomToTop(sptr<WindowNode> node, const WindowNodeOperationFunc& func) const
{
    if (node == nullptr) {
        return false;
    }
    auto iterBegin = node->children_.begin();
    for (; iterBegin != node->children_.end(); ++iterBegin) {
        if ((*iterBegin)->priority_ >= 0) {
            break;
        }
        if (func(*iterBegin)) {
            return true;
        }
    }
    if (func(node)) {
        return true;
    }
    for (; iterBegin != node->children_.end(); ++iterBegin) {
        if (func(*iterBegin)) {
            return true;
        }
    }
    return false;
}

Rect WindowNodeContainer::GetDisplayGroupRect() const
{
    return layoutPolicy_->GetDisplayGroupRect();
}

void WindowNodeContainer::UpdateSizeChangeReason(sptr<WindowNode>& node, WindowMode srcMode, WindowMode dstMode)
{
    if ((srcMode == WindowMode::WINDOW_MODE_FULLSCREEN) && (dstMode == WindowMode::WINDOW_MODE_FLOATING)) {
        node->SetWindowSizeChangeReason(WindowSizeChangeReason::RECOVER);
    } else if (dstMode == WindowMode::WINDOW_MODE_FULLSCREEN) {
        node->SetWindowSizeChangeReason(WindowSizeChangeReason::MAXIMIZE);
        if (srcMode == WindowMode::WINDOW_MODE_FLOATING) {
            node->SetRequestRect(node->GetWindowRect());
        }
    } else if (WindowHelper::IsFullScreenWindow(srcMode) && WindowHelper::IsSplitWindowMode(dstMode)) {
        node->SetWindowSizeChangeReason(WindowSizeChangeReason::FULL_TO_SPLIT);
    } else if (WindowHelper::IsSplitWindowMode(srcMode) && WindowHelper::IsFullScreenWindow(dstMode)) {
        node->SetWindowSizeChangeReason(WindowSizeChangeReason::SPLIT_TO_FULL);
    } else {
        node->SetWindowSizeChangeReason(WindowSizeChangeReason::RESIZE);
    }
}

WMError WindowNodeContainer::SetWindowMode(sptr<WindowNode>& node, WindowMode dstMode)
{
    if (node == nullptr) {
        WLOGFE("could not find window");
        return WMError::WM_ERROR_NULLPTR;
    }
    WindowMode srcMode = node->GetWindowMode();
    if (WindowHelper::IsSplitWindowMode(dstMode) && isScreenLocked_ &&
        (node->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED))) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    auto windowPair = displayGroupController_->GetWindowPairByDisplayId(node->GetDisplayId());
    if (windowPair == nullptr) {
        WLOGFE("Window pair is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }

    WindowPairStatus status = windowPair->GetPairStatus();
    // when status is single primary or single secondary, split node is abandoned to set mode
    if (node->IsSplitMode() && (status == WindowPairStatus::SINGLE_PRIMARY ||
        status == WindowPairStatus::SINGLE_SECONDARY)) {
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    WMError res = WMError::WM_OK;
    UpdateSizeChangeReason(node, srcMode, dstMode);
    node->SetWindowMode(dstMode);
    windowPair->UpdateIfSplitRelated(node);

    if (WindowHelper::IsMainWindow(node->GetWindowType())) {
        if (WindowHelper::IsFloatingWindow(node->GetWindowMode())) {
            NotifyDockWindowStateChanged(node, true);
        } else {
            NotifyDockWindowStateChanged(node, false);
        }
    }

    if (node->GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN &&
        WindowHelper::IsAppWindow(node->GetWindowType())) {
        // minimize other app window.
        res = MinimizeStructuredAppWindowsExceptSelf(node);
        if (res != WMError::WM_OK) {
            return res;
        }
    }
    // when change mode, need to reset shadow and radius.
    WindowSystemEffect::SetWindowEffect(node);

    // when change mode, need to reset MainFloatingWindow ZOrder.
    ResetWindowZOrderPriorityWhenSetMode(node, dstMode, srcMode);
    MinimizeOldestMainFloatingWindow(node->GetWindowId());

    if (node->GetWindowToken() != nullptr) {
        node->GetWindowToken()->UpdateWindowMode(node->GetWindowMode());
    }
    res = UpdateWindowNode(node, WindowUpdateReason::UPDATE_MODE);
    if (res != WMError::WM_OK) {
        WLOGFE("Set window mode failed, update node failed");
        return res;
    }
    return WMError::WM_OK;
}

void WindowNodeContainer::ResetWindowZOrderPriorityWhenSetMode(sptr<WindowNode>& node,
    const WindowMode& dstMode, const WindowMode& srcMode)
{
    if (!isFloatWindowAboveFullWindow_) {
        return;
    }

    // reset node zorder priority.
    if (WindowHelper::IsMainFloatingWindow(node->GetWindowType(), srcMode)) {
        auto basePriority = zorderPolicy_->GetWindowPriority(node->GetWindowType());
        if (isScreenLocked_ &&
            (node->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED))) {
            basePriority = zorderPolicy_->GetWindowPriority(WindowType::WINDOW_TYPE_KEYGUARD) + 1;
        }
        node->priority_ = basePriority;
    }

    if (!WindowHelper::IsMainFloatingWindow(node->GetWindowType(), srcMode) &&
        !WindowHelper::IsMainFloatingWindow(node->GetWindowType(), dstMode) &&
        !WindowHelper::IsSplitWindowMode(srcMode) &&
        !WindowHelper::IsSplitWindowMode(dstMode)) {
        return;
    }

    // When set mode, all floating window should be checked and raise
    ResetAllMainFloatingWindowZOrder(node->parent_);
    if (node->parent_ != nullptr && node->GetWindowId() == focusedWindow_ &&
        WindowHelper::IsMainFloatingWindow(node->GetWindowType(), dstMode)) {
        // if current node is mainFloatingWIndow and foucsedWindow, it should be raised to top.
        RaiseWindowToTop(node->GetWindowId(), node->parent_->children_);
    }
    AssignZOrder();
}

void WindowNodeContainer::GetModeChangeHotZones(DisplayId displayId, ModeChangeHotZones& hotZones,
    const ModeChangeHotZonesConfig& config)
{
    const auto& displayRect = DisplayGroupInfo::GetInstance().GetDisplayRect(displayId);

    hotZones.fullscreen_.width_ = displayRect.width_;
    hotZones.fullscreen_.height_ = config.fullscreenRange_;

    hotZones.primary_.width_ = config.primaryRange_;
    hotZones.primary_.height_ = displayRect.height_;

    hotZones.secondary_.posX_ = static_cast<int32_t>(displayRect.width_) - config.secondaryRange_;
    hotZones.secondary_.width_ = config.secondaryRange_;
    hotZones.secondary_.height_ = displayRect.height_;
}

void WindowNodeContainer::UpdateCameraFloatWindowStatus(const sptr<WindowNode>& node, bool isShowing)
{
    if (node->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT_CAMERA) {
        WindowManagerAgentController::GetInstance().UpdateCameraFloatWindowStatus(node->GetAccessTokenId(), isShowing);
    }
}

WindowLayoutMode WindowNodeContainer::GetCurrentLayoutMode() const
{
    return layoutMode_;
}

void WindowNodeContainer::RemoveSingleUserWindowNodes(int accountId)
{
    std::vector<sptr<WindowNode>> windowNodes;
    TraverseContainer(windowNodes);
    WLOGI("%{public}d", accountId);
    for (auto& windowNode : windowNodes) {
        int windowAccountId = windowNode->GetCallingUid() / UID_TRANSFROM_DIVISOR;
        if (windowAccountId < UID_MIN || windowAccountId == accountId) {
            WLOGD("skiped window %{public}s, windowId %{public}d uid %{public}d",
                windowNode->GetWindowName().c_str(), windowNode->GetWindowId(), windowNode->GetCallingUid());
            continue;
        }
        WLOGD("remove window %{public}s, windowId %{public}d uid %{public}d",
            windowNode->GetWindowName().c_str(), windowNode->GetWindowId(), windowNode->GetCallingUid());
        windowNode->GetWindowProperty()->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::NONE));
        if (windowNode->GetWindowToken()) {
            if (windowNode->surfaceNode_ != nullptr) {
                windowNode->surfaceNode_->SetVisible(true);
            }
            windowNode->GetWindowToken()->UpdateWindowState(WindowState::STATE_HIDDEN);
        }
    }
}

bool WindowNodeContainer::TakeWindowPairSnapshot(DisplayId displayId)
{
    auto windowPair = displayGroupController_->GetWindowPairByDisplayId(displayId);
    return windowPair == nullptr ? false : windowPair->TakePairSnapshot();
}

void WindowNodeContainer::ClearWindowPairSnapshot(DisplayId displayId)
{
    auto windowPair = displayGroupController_->GetWindowPairByDisplayId(displayId);
    if (windowPair == nullptr) {
        WLOGFE("Window pair is nullptr");
        return;
    }
    windowPair->ClearPairSnapshot();
}

void WindowNodeContainer::SetWindowPairFrameGravity(DisplayId displayId, Gravity gravity)
{
    auto windowPair = displayGroupController_->GetWindowPairByDisplayId(displayId);
    if (windowPair == nullptr) {
        WLOGFE("Window pair is nullptr");
        return;
    }
    std::vector<sptr<WindowNode>> windowNodes = windowPair->GetPairedWindows();
    for (auto& windowNode : windowNodes) {
        if (windowNode->surfaceNode_) {
            windowNode->surfaceNode_->SetFrameGravity(gravity);
        }
    }
}

bool WindowNodeContainer::IsScreenLocked()
{
    return isScreenLocked_;
}

bool WindowNodeContainer::GetAnimateTransactionEnabled()
{
    return isAnimateTransactionEnabled_;
}
} // namespace Rosen
} // namespace OHOS
