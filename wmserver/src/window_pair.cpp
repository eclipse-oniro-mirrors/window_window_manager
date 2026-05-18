/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "window_pair.h"

#include <ability_manager_client.h>
#include "common_event_manager.h"
#include "minimize_app.h"
#include "window_inner_manager.h"
#include "window_manager_hilog.h"
#include "window_helper.h"
#include "window_system_effect.h"
#include "surface_draw.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "Pair"};
const std::string SPLIT_SCREEN_EVENT_NAME = "common.event.SPLIT_SCREEN";
const std::map<SplitEventMsgType, std::string> splitEventDataMap {
    {SplitEventMsgType::MSG_SHOW_PRIMARY,                                           "Primary"},
    {SplitEventMsgType::MSG_SHOW_SECONDARY,                                       "Secondary"},
    {SplitEventMsgType::MSG_SHOW_DIVIDER,       "common.event.SPLIT_SCREEN.data.show.divider"},
    {SplitEventMsgType::MSG_DESTROY_DIVIDER, "common.event.SPLIT_SCREEN.data.destroy.divider"}
};
}

WindowPair::~WindowPair()
{
    WLOGD("~WindowPair");
    Clear();
}

void WindowPair::SendSplitScreenCommonEvent(SplitEventMsgType msgType, int32_t missionId)
{
    std::string data = splitEventDataMap.at(msgType);
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    AAFwk::Want want;
    want.SetAction(SPLIT_SCREEN_EVENT_NAME);
    want.SetParam("windowMode", data);
    want.SetParam("missionId", missionId);
    EventFwk::CommonEventData commonEventData;
    commonEventData.SetWant(want);
    EventFwk::CommonEventManager::PublishCommonEvent(commonEventData);
    // set ipc identity to raw
    IPCSkeleton::SetCallingIdentity(identity);
    WLOGD("Send split screen event: %{public}s", data.c_str());
}

void WindowPair::NotifyShowRecent(sptr<WindowNode> node)
{
    if (node == nullptr) {
        return;
    }
    auto msgType = (node->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) ?
        SplitEventMsgType::MSG_SHOW_PRIMARY : SplitEventMsgType::MSG_SHOW_SECONDARY;
    SendSplitScreenCommonEvent(msgType, node->abilityInfo_.missionId_);
}

void WindowPair::NotifyCreateOrDestroyDivider(sptr<WindowNode> node, bool isDestroy)
{
    if (node == nullptr) {
        return;
    }
    auto msgType = isDestroy ? SplitEventMsgType::MSG_DESTROY_DIVIDER : SplitEventMsgType::MSG_SHOW_DIVIDER;
    SendSplitScreenCommonEvent(msgType, node->abilityInfo_.missionId_);
}

sptr<WindowNode> WindowPair::Find(sptr<WindowNode>& node)
{
    if (node == nullptr) {
        return nullptr;
    }
    if (primary_ != nullptr && primary_->GetWindowId() == node->GetWindowId()) {
        return primary_;
    } else if (secondary_ != nullptr && secondary_->GetWindowId() == node->GetWindowId()) {
        return secondary_;
    } else if (divider_ != nullptr && divider_->GetWindowId() == node->GetWindowId()) {
        return divider_;
    }
    return nullptr;
}

bool WindowPair::IsPaired() const
{
    if (primary_ == nullptr || secondary_ == nullptr) {
        return false;
    }
    if (primary_->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY &&
        secondary_->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY &&
        divider_ != nullptr) {
        return true;
    }
    return false;
}

bool WindowPair::IsAbnormalStatus() const
{
    if (status_ == WindowPairStatus::SINGLE_SPLIT || status_ == WindowPairStatus::PRIMARY_AND_DIVIDER ||
        status_ == WindowPairStatus::SECONDARY_AND_DIVIDER) {
        return true;
    } else {
        return false;
    }
}

void WindowPair::SetSplitRatio(float ratio)
{
    ratio_ = ratio;
}

float WindowPair::GetSplitRatio() const
{
    return ratio_;
}

WindowPairStatus WindowPair::GetPairStatus() const
{
    return status_;
}

sptr<WindowNode> WindowPair::GetDividerWindow() const
{
    return divider_;
}

bool WindowPair::IsForbidDockSliceMove() const
{
    if (status_ != WindowPairStatus::PAIRED_DONE) {
        return false;
    }
    uint32_t flag = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE);
    if (primary_ != nullptr && !(primary_->GetWindowFlags() & flag) && secondary_ != nullptr &&
        !(secondary_->GetWindowFlags() & flag)) {
        return false;
    }
    return true;
}

bool WindowPair::IsDockSliceInExitSplitModeArea(const std::vector<int32_t>& exitSplitPoints)
{
    if (!IsPaired()) {
        return false;
    }
    int32_t dividerOrigin;
    Rect rect = divider_->GetWindowRect();
    if (rect.width_ < rect.height_) {
        dividerOrigin = rect.posX_;
    } else {
        dividerOrigin = rect.posY_; // vertical display
    }
    if (dividerOrigin < exitSplitPoints[0] || dividerOrigin > exitSplitPoints[1]) {
        return true;
    }
    return false;
}

void WindowPair::ExitSplitMode()
{
    if (!IsPaired()) {
        return;
    }
    Rect dividerRect = divider_->GetWindowRect();
    sptr<WindowNode> hideNode, recoveryNode;
    bool isVertical = (dividerRect.height_ < dividerRect.width_) ? true : false;
    if ((isVertical && (primary_->GetWindowRect().height_ < secondary_->GetWindowRect().height_)) ||
        (!isVertical && (primary_->GetWindowRect().width_ < secondary_->GetWindowRect().width_))) {
        hideNode = primary_;
        recoveryNode = secondary_;
    } else {
        hideNode = secondary_;
        recoveryNode = primary_;
    }
    if (recoveryNode != nullptr) {
        recoveryNode->SetSnapshot(nullptr);
    }
    MinimizeApp::AddNeedMinimizeApp(hideNode, MinimizeReason::SPLIT_QUIT);
    MinimizeApp::ExecuteMinimizeTargetReasons(MinimizeReason::SPLIT_QUIT);
    WLOGI("Exit Split Mode, Minimize Window %{public}u", hideNode->GetWindowId());
}

void WindowPair::Clear()
{
    WLOGI("Clear window pair.");
    DumpPairInfo();
    auto splitMode = (WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY |
                      WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY);
    if (primary_ != nullptr && primary_->GetWindowProperty() != nullptr &&
        primary_->GetWindowToken() != nullptr) {
        if (primary_->GetWindowModeSupportType() == splitMode) {
            MinimizeApp::AddNeedMinimizeApp(primary_, MinimizeReason::SPLIT_QUIT);
            MinimizeApp::ExecuteMinimizeTargetReasons(MinimizeReason::SPLIT_QUIT);
        } else {
            if (WindowHelper::IsFullScreenWindow(primary_->GetWindowProperty()->GetLastWindowMode()) &&
                WindowHelper::IsSplitWindowMode(primary_->GetWindowProperty()->GetWindowMode()) &&
                primary_->GetWindowType() != WindowType::WINDOW_TYPE_LAUNCHER_RECENT) {
                primary_->SetWindowSizeChangeReason(WindowSizeChangeReason::SPLIT_TO_FULL);
            }
            primary_->GetWindowProperty()->ResumeLastWindowMode();
            // when change mode, need to reset shadow and radius
            WindowSystemEffect::SetWindowEffect(primary_);
            primary_->GetWindowToken()->UpdateWindowMode(WindowModeInfo{ primary_->GetWindowMode() });
        }
    }
    if (secondary_ != nullptr && secondary_->GetWindowProperty() != nullptr &&
        secondary_->GetWindowToken() != nullptr) {
        if (secondary_->GetWindowModeSupportType() == splitMode) {
            MinimizeApp::AddNeedMinimizeApp(secondary_, MinimizeReason::SPLIT_QUIT);
            MinimizeApp::ExecuteMinimizeTargetReasons(MinimizeReason::SPLIT_QUIT);
        } else {
            if (WindowHelper::IsFullScreenWindow(secondary_->GetWindowProperty()->GetLastWindowMode()) &&
                WindowHelper::IsSplitWindowMode(secondary_->GetWindowProperty()->GetWindowMode()) &&
                secondary_->GetWindowType() != WindowType::WINDOW_TYPE_LAUNCHER_RECENT) {
                secondary_->SetWindowSizeChangeReason(WindowSizeChangeReason::SPLIT_TO_FULL);
            }
            secondary_->GetWindowProperty()->ResumeLastWindowMode();
            // when change mode, need to reset shadow and radius
            WindowSystemEffect::SetWindowEffect(secondary_);
            secondary_->GetWindowToken()->UpdateWindowMode(WindowModeInfo{ secondary_->GetWindowMode() });
        }
    }

    primary_ = nullptr;
    secondary_ = nullptr;
    if (divider_ != nullptr) {
        NotifyCreateOrDestroyDivider(divider_, true);
        divider_ = nullptr;
    }
    status_ = WindowPairStatus::EMPTY;
}

bool WindowPair::IsSplitRelated(sptr<WindowNode>& node) const
{
    if (node == nullptr) {
        return false;
    }
    return WindowHelper::IsSplitWindowMode((node->GetWindowMode())) ||
        (node->GetWindowType() == WindowType::WINDOW_TYPE_DOCK_SLICE);
}

void WindowPair::CheckOrderedPairZorder(
    sptr<WindowNode>& node, bool& hasPrimaryDialog, bool& hasSecondaryDialog, bool& isPrimaryAbove)
{
    if (primary_ != nullptr) {
        for (auto& child : primary_->children_) {
            if (child->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) {
                // secondary divider primary
                hasPrimaryDialog = true;
                isPrimaryAbove = true;
                break;
            }
        }
    }
    if (secondary_ != nullptr) {
        for (auto& child : secondary_->children_) {
            if (child->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) {
                // primary divider secondary
                hasSecondaryDialog = true;
                isPrimaryAbove = false;
                break;
            }
        }
    }
    if (node->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY ||
        node->GetWindowType() == WindowType::WINDOW_TYPE_DOCK_SLICE) {
        // primary secondary divider
        isPrimaryAbove = false;
    } else if (node->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY) {
        // secondary primary divider
        isPrimaryAbove = true;
    }

    return;
}

std::vector<sptr<WindowNode>> WindowPair::CreateOrderedPair(
    sptr<WindowNode>& bottom, sptr<WindowNode>& mid, sptr<WindowNode>& top)
{
    std::vector<sptr<WindowNode>> orderedPair;

    if (bottom != nullptr) {
        orderedPair.push_back(bottom);
    }
    if (mid != nullptr) {
        orderedPair.push_back(mid);
    }
    if (top != nullptr) {
        orderedPair.push_back(top);
    }

    return orderedPair;
}

std::vector<sptr<WindowNode>> WindowPair::GetOrderedPair(sptr<WindowNode>& node)
{
    WLOGI("Get paired node in Z order");
    std::vector<sptr<WindowNode>> orderedPair;
    bool hasPrimaryDialog_ = false;
    bool hasSecondaryDialog_ = false;
    bool isPrimaryAbove_ = false;

    if (node == nullptr || Find(node) == nullptr) {
        return orderedPair;
    }

    CheckOrderedPairZorder(node, hasPrimaryDialog_, hasSecondaryDialog_, isPrimaryAbove_);

    if (hasPrimaryDialog_ && hasSecondaryDialog_) {
        return CreateOrderedPair(divider_, primary_, secondary_);
    }
    if (hasPrimaryDialog_ || hasSecondaryDialog_) {
        if (isPrimaryAbove_) {
            return CreateOrderedPair(secondary_, divider_, primary_);
        } else {
            return CreateOrderedPair(primary_, divider_, secondary_);
        }
    } else {
        if (isPrimaryAbove_) {
            return CreateOrderedPair(secondary_, primary_, divider_);
        } else {
            return CreateOrderedPair(primary_, secondary_, divider_);
        }
    }
}

std::vector<sptr<WindowNode>> WindowPair::GetPairedWindows()
{
    WLOGD("Get primary and secondary of window pair");
    std::vector<sptr<WindowNode>> pairWindows;
    if (status_ == WindowPairStatus::PAIRED_DONE && primary_ != nullptr && secondary_ != nullptr) {
        pairWindows = {primary_, secondary_};
    }
    return pairWindows;
}

bool WindowPair::StatusSupprtedWhenRecentUpdate(sptr<WindowNode>& node)
{
    WindowMode recentMode_ = node->GetWindowMode();
    if (recentMode_ == WindowMode::WINDOW_MODE_SPLIT_PRIMARY &&
       (status_ == WindowPairStatus::SINGLE_SECONDARY || status_ == WindowPairStatus::SECONDARY_AND_DIVIDER)) {
        return true;
    } else if (recentMode_ == WindowMode::WINDOW_MODE_SPLIT_SECONDARY &&
        (status_ == WindowPairStatus::SINGLE_PRIMARY || status_ == WindowPairStatus::PRIMARY_AND_DIVIDER)) {
        return true;
    }
    return false;
}

void WindowPair::UpdateIfSplitRelated(sptr<WindowNode>& node)
{
    if (node == nullptr) {
        return;
    }
    if (Find(node) == nullptr && !IsSplitRelated(node)) {
        WLOGFD("Window id: %{public}u is not split related and paired.", node->GetWindowId());
        return;
    }
    if ((node->GetWindowType() == WindowType::WINDOW_TYPE_PLACEHOLDER) &&
        ((primary_ != nullptr && primary_->GetWindowMode() == node->GetWindowMode()) ||
        (secondary_ != nullptr && secondary_->GetWindowMode() == node->GetWindowMode()))) {
        WindowInnerManager::GetInstance().DestroyInnerWindow(displayId_, WindowType::WINDOW_TYPE_PLACEHOLDER);
        return;
    }
    WLOGI("Current status: %{public}u, window id: %{public}u mode: %{public}u",
        status_, node->GetWindowId(), node->GetWindowMode());
    // when status not support to start recent, clear split node and return
    if (node->GetWindowType() == WindowType::WINDOW_TYPE_LAUNCHER_RECENT && node->IsSplitMode() &&
        !StatusSupprtedWhenRecentUpdate(node)) {
        Clear();
        return;
    }
    if (status_ == WindowPairStatus::EMPTY) {
        Insert(node);
        if (!isAllSplitAppWindowsRestoring_) {
            WindowMode holderMode = node->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ?
                WindowMode::WINDOW_MODE_SPLIT_SECONDARY : WindowMode::WINDOW_MODE_SPLIT_PRIMARY;
            WindowInnerManager::GetInstance().CreateInnerWindow("place_holder", displayId_, DEFAULT_PLACE_HOLDER_RECT,
                WindowType::WINDOW_TYPE_PLACEHOLDER, holderMode);
            // notity systemui to create divider window
            NotifyShowRecent(node);
        }
    } else {
        if (Find(node) == nullptr) {
            // add new split related node to pair
            Insert(node);
        } else {
            // handle paired nodes change
            HandlePairedNodesChange();
        }
    }
}

void WindowPair::UpdateWindowPairStatus()
{
    WLOGI("Update window pair status.");
    WindowPairStatus prevStatus = status_;
    if (primary_ != nullptr && secondary_ != nullptr && divider_ != nullptr) {
        status_ = WindowPairStatus::PAIRED_DONE;
    } else if (primary_ != nullptr && secondary_ != nullptr && divider_ == nullptr) {
        status_ = WindowPairStatus::PRIMARY_AND_SECONDARY;
    } else if (primary_ != nullptr && secondary_ == nullptr && divider_ == nullptr) {
        status_ = WindowPairStatus::SINGLE_PRIMARY;
    } else if (primary_ != nullptr && secondary_ == nullptr && divider_ != nullptr) {
        status_ = WindowPairStatus::PRIMARY_AND_DIVIDER;
    } else if (primary_ == nullptr && secondary_ != nullptr && divider_ == nullptr) {
        status_ = WindowPairStatus::SINGLE_SECONDARY;
    } else if (primary_ == nullptr && secondary_ != nullptr && divider_ != nullptr) {
        status_ = WindowPairStatus::SECONDARY_AND_DIVIDER;
    } else if (primary_ == nullptr && secondary_ == nullptr && divider_ != nullptr) {
        status_ = WindowPairStatus::SINGLE_SPLIT;
    } else {
        status_ = WindowPairStatus::EMPTY;
    }
    if ((prevStatus == WindowPairStatus::SINGLE_PRIMARY ||
        prevStatus == WindowPairStatus::SINGLE_SECONDARY || prevStatus == WindowPairStatus::EMPTY) &&
        status_ == WindowPairStatus::PRIMARY_AND_SECONDARY) {
        // notify systemui to create divider
        NotifyCreateOrDestroyDivider(primary_, false);
    } else if ((prevStatus == WindowPairStatus::PAIRED_DONE || prevStatus == WindowPairStatus::PRIMARY_AND_SECONDARY) &&
        (status_ != WindowPairStatus::PAIRED_DONE && status_ != WindowPairStatus::PRIMARY_AND_SECONDARY)) {
        Clear();
    }
    DumpPairInfo();
}

void WindowPair::SwitchPosition()
{
    if (primary_ == nullptr || secondary_ == nullptr) {
        return;
    }
    WLOGI("Switch the pair pos, pri: %{public}u pri-mode: %{public}u, sec: %{public}u sec-mode: %{public}u,",
        primary_->GetWindowId(), primary_->GetWindowMode(), secondary_->GetWindowId(), secondary_->GetWindowMode());
    if (primary_->GetWindowMode() == secondary_->GetWindowMode() &&
        primary_->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY) {
        primary_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
        // when change mode, need to reset shadow and radius
        WindowSystemEffect::SetWindowEffect(primary_);
        if (primary_->GetWindowToken() != nullptr) {
            primary_->GetWindowToken()->UpdateWindowMode(WindowModeInfo{ WindowMode::WINDOW_MODE_SPLIT_SECONDARY });
        }
        std::swap(primary_, secondary_);
    } else if (primary_->GetWindowMode() == secondary_->GetWindowMode() &&
        primary_->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) {
        secondary_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
        // when change mode, need to reset shadow and radius
        WindowSystemEffect::SetWindowEffect(secondary_);
        if (secondary_->GetWindowToken() != nullptr) {
            secondary_->GetWindowToken()->UpdateWindowMode(WindowModeInfo{ WindowMode::WINDOW_MODE_SPLIT_PRIMARY });
        }
        std::swap(primary_, secondary_);
    }
}

void WindowPair::HandlePairedNodesChange()
{
    WLOGI("Update pair node.");
    if (primary_ != nullptr && !primary_->IsSplitMode()) {
        primary_ = nullptr;
    }
    if (secondary_ != nullptr && !secondary_->IsSplitMode()) {
        secondary_ = nullptr;
    }
    // paired node mode change
    if (primary_ != nullptr && secondary_ == nullptr &&
        primary_->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) {
        std::swap(primary_, secondary_);
    } else if (primary_ == nullptr && secondary_ != nullptr &&
        secondary_->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY) {
        std::swap(primary_, secondary_);
    } else if (primary_ != nullptr && secondary_ != nullptr &&
        primary_->GetWindowMode() == secondary_->GetWindowMode()) {
        // switch position
        SwitchPosition();
    }
    UpdateWindowPairStatus();
    if (IsAbnormalStatus()) {
        Clear();
    }
}

void WindowPair::Insert(sptr<WindowNode>& node)
{
    if (node == nullptr) {
        return;
    }
    WLOGI("Insert a window to pair id: %{public}u", node->GetWindowId());
    sptr<WindowNode> pairedNode;
    if (node->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY) {
        pairedNode = primary_;
        primary_ = node;
    } else if (node->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) {
        pairedNode = secondary_;
        secondary_ = node;
    } else if (node->GetWindowType() == WindowType::WINDOW_TYPE_DOCK_SLICE) {
        pairedNode = divider_;
        divider_ = node;
    }
    // minimize invalid paired window
    if (pairedNode != nullptr && pairedNode->abilityToken_ != nullptr) {
        MinimizeApp::AddNeedMinimizeApp(pairedNode, MinimizeReason::SPLIT_REPLACE);
    }
    UpdateWindowPairStatus();
    if (IsAbnormalStatus()) {
        Clear();
    }
}

void WindowPair::DumpPairInfo()
{
    if (primary_ != nullptr) {
        WLOGI("[DumpPairInfo] primary id: %{public}u mode: %{public}u", primary_->GetWindowId(),
            primary_->GetWindowMode());
    }
    if (secondary_ != nullptr) {
        WLOGI("[DumpPairInfo] secondary id: %{public}u mode: %{public}u", secondary_->GetWindowId(),
            secondary_->GetWindowMode());
    }
    if (divider_ != nullptr) {
        WLOGI("[DumpPairInfo] divider id: %{public}u mode: %{public}u", divider_->GetWindowId(),
            divider_->GetWindowMode());
    }
    WLOGI("[DumpPairInfo] pair status %{public}u", status_);
}

void WindowPair::HandleRemoveWindow(sptr<WindowNode>& node)
{
    if (node == nullptr) {
        return;
    }
    if (Find(node) == nullptr && node->IsSplitMode()) {
        WLOGI("Resume unpaired split related window id: %{public}u", node->GetWindowId());
        if (node->GetWindowProperty() != nullptr && node->GetWindowToken() != nullptr) {
            node->GetWindowProperty()->ResumeLastWindowMode();
            // when change mode, need to reset shadow and radius
            WindowSystemEffect::SetWindowEffect(node);
            node->GetWindowToken()->UpdateWindowMode(WindowModeInfo{ node->GetWindowMode() });
        }
        // target node is not in window pair, need resume mode when remove
        return;
    } else if (Find(node) != nullptr) {
        WLOGI("Pairing window id: %{public}u is remove, clear window pair", node->GetWindowId());
        Clear();
    }
}

void WindowPair::RotateDividerWindow(const Rect& rect)
{
    dividerRect_ = rect;
    // rotate divider when display orientation changed
    if (divider_ == nullptr) {
        WLOGE("Rotate divider failed because divider is null");
        return;
    }
    WLOGFD("Rotate divider when display rotate rect:[%{public}d, %{public}d, %{public}u, %{public}u]",
        rect.posX_, rect.posY_, rect.width_, rect.height_);
}

void WindowPair::SetDividerRect(const Rect& rect)
{
    dividerRect_ = rect;
}

bool WindowPair::TakePairSnapshot()
{
    if (status_ == WindowPairStatus::PAIRED_DONE && primary_ != nullptr && secondary_ != nullptr) {
        WLOGD("Take pair snapshot id:[%{public}u, %{public}u]", primary_->GetWindowId(), secondary_->GetWindowId());
        std::shared_ptr<Media::PixelMap> pixelMap;
        // get pixelmap time out 2000ms
        if (SurfaceDraw::GetSurfaceSnapshot(primary_->surfaceNode_, pixelMap, SNAPSHOT_TIMEOUT_MS)) {
            primary_->SetSnapshot(pixelMap);
        }
        // get pixelmap time out 2000ms
        if (SurfaceDraw::GetSurfaceSnapshot(secondary_->surfaceNode_, pixelMap, SNAPSHOT_TIMEOUT_MS)) {
            secondary_->SetSnapshot(pixelMap);
        }
        return true;
    }
    return false;
}

void WindowPair::ClearPairSnapshot()
{
    WLOGD("Clear window pair snapshot");
    if (primary_ != nullptr) {
        primary_->SetSnapshot(nullptr);
    }
    if (secondary_ != nullptr) {
        secondary_->SetSnapshot(nullptr);
    }
}

int32_t WindowPair::GetSplitRatioPoint(float ratio, const Rect& displayRect)
{
    if (displayRect.width_ > displayRect.height_) {
        return displayRect.posX_ +
            static_cast<uint32_t>((displayRect.width_ - dividerRect_.width_) * ratio);
    } else {
        return displayRect.posY_ +
            static_cast<uint32_t>((displayRect.height_ - dividerRect_.height_) * ratio);
    }
}

void WindowPair::CalculateSplitRatioPoints(const Rect& displayRect)
{
    exitSplitPoints_.clear();
    splitRatioPoints_.clear();
    exitSplitPoints_.push_back(GetSplitRatioPoint(splitRatioConfig_.exitSplitStartRatio, displayRect));
    exitSplitPoints_.push_back(GetSplitRatioPoint(splitRatioConfig_.exitSplitEndRatio, displayRect));
    for (const auto& ratio : splitRatioConfig_.splitRatios) {
        splitRatioPoints_.push_back(GetSplitRatioPoint(ratio, displayRect));
    }
}

void WindowPair::SetSplitRatioConfig(const SplitRatioConfig& splitRatioConfig)
{
    splitRatioConfig_ = splitRatioConfig;
}

std::vector<int32_t> WindowPair::GetExitSplitPoints()
{
    return exitSplitPoints_;
}

std::vector<int32_t> WindowPair::GetSplitRatioPoints()
{
    return splitRatioPoints_;
}

bool WindowPair::IsDuringSplit()
{
    if (status_ == WindowPairStatus::EMPTY) {
        return false;
    }

    if (status_ != WindowPairStatus::PAIRED_DONE || primary_ == nullptr || secondary_ == nullptr) {
        WLOGFD("missing pairWindows or split status is %{public}u not done", status_);
        return true;
    }

    if (primary_->GetWindowType() == WindowType::WINDOW_TYPE_LAUNCHER_RECENT ||
        secondary_->GetWindowType() == WindowType::WINDOW_TYPE_LAUNCHER_RECENT) {
        WLOGFD("split is done, but there is recent");
        return true;
    }

    return false;
}
} // namespace Rosen
} // namespace OHOS