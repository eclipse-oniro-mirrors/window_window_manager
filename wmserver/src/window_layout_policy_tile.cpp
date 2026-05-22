/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "window_layout_policy_tile.h"
#include <ability_manager_client.h>
#include <hitrace_meter.h>

#include "minimize_app.h"
#include "window_helper.h"
#include "window_inner_manager.h"
#include "window_manager_hilog.h"
#include "window_system_effect.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "Tile"};
constexpr uint32_t EDGE_INTERVAL = 48;
constexpr uint32_t MID_INTERVAL = 24;
}

WindowLayoutPolicyTile::WindowLayoutPolicyTile(DisplayGroupWindowTree& displayGroupWindowTree)
    : WindowLayoutPolicy(displayGroupWindowTree)
{
}

void WindowLayoutPolicyTile::Launch()
{
    for (auto& iter : DisplayGroupInfo::GetInstance().GetAllDisplayRects()) {
        const auto& displayId = iter.first;
        /*
         * Init tile rects and layout tile queue
         */
        InitTileRects(displayId);
        InitTileQueue(displayId);
        LayoutTileQueue(displayId);

        /*
         * Layout above and below nodes, it is necessary when display rotatation or size change
         */
        auto& displayWindowTree = displayGroupWindowTree_[displayId];
        LayoutWindowNodesByRootType(*(displayWindowTree[WindowRootNodeType::ABOVE_WINDOW_NODE]));
        LayoutWindowNodesByRootType(*(displayWindowTree[WindowRootNodeType::BELOW_WINDOW_NODE]));
        WLOGFD("[Launch TileLayout], displayId: %{public}" PRIu64"", displayId);
    }
    WLOGI("[Launch TileLayout Finished]");
}

uint32_t WindowLayoutPolicyTile::GetMaxTileWinNum(DisplayId displayId) const
{
    float virtualPixelRatio = DisplayGroupInfo::GetInstance().GetDisplayVirtualPixelRatio(displayId);
    constexpr uint32_t half = 2;
    uint32_t edgeIntervalVp = static_cast<uint32_t>(EDGE_INTERVAL * half * virtualPixelRatio);
    uint32_t midIntervalVp = static_cast<uint32_t>(MID_INTERVAL * virtualPixelRatio);
    uint32_t minFloatingW = static_cast<uint32_t>(MIN_FLOATING_WIDTH * virtualPixelRatio);
    uint32_t drawableW = limitRectMap_[displayId].width_ - edgeIntervalVp + midIntervalVp;
    return static_cast<uint32_t>(drawableW / (minFloatingW + midIntervalVp));
}

void WindowLayoutPolicyTile::InitTileRects(DisplayId displayId)
{
    // TileLayout don't consider limitRect yet, limitDisplay equals to displayRect
    const auto& displayRect = DisplayGroupInfo::GetInstance().GetDisplayRect(displayId);
    if (WindowHelper::IsEmptyRect(displayRect)) {
        WLOGFE("DisplayRect is empty, displayRect: %{public}" PRIu64"", displayId);
        return;
    }

    limitRectMap_[displayId] = displayRect;
    float virtualPixelRatio = DisplayGroupInfo::GetInstance().GetDisplayVirtualPixelRatio(displayId);
    uint32_t edgeIntervalVp = static_cast<uint32_t>(EDGE_INTERVAL * virtualPixelRatio);
    uint32_t midIntervalVp = static_cast<uint32_t>(MID_INTERVAL * virtualPixelRatio);

    constexpr float ratio = DEFAULT_ASPECT_RATIO;
    const Rect& limitRect = limitRectMap_[displayId];
    constexpr int half = 2;
    maxTileWinNumMap_[displayId] = GetMaxTileWinNum(displayId);
    WLOGFD("set max tile window num %{public}u", maxTileWinNumMap_[displayId]);
    auto& presetRectsForAllLevel = presetRectsMap_[displayId];
    presetRectsForAllLevel.clear();
    uint32_t w = displayRect.width_ * ratio;
    uint32_t h = displayRect.height_ * ratio;
    w = w > limitRect.width_ ? limitRect.width_ : w;
    h = h > limitRect.height_ ? limitRect.height_ : h;
    int32_t x = limitRect.posX_ + (static_cast<int32_t>(limitRect.width_ - w) / half);
    int32_t y = limitRect.posY_ + (static_cast<int32_t>(limitRect.height_ - h) / half);

    std::vector<Rect> single = {{ x, y, w, h }};
    presetRectsForAllLevel.emplace_back(single);
    for (uint32_t num = 2; num <= maxTileWinNumMap_[displayId]; num++) { // start calc preset with 2 windows
        w = (limitRect.width_ - edgeIntervalVp * half - midIntervalVp * (num - 1)) / num;
        std::vector<Rect> curLevel;
        for (uint32_t i = 0; i < num; i++) {
            int32_t curX = static_cast<int32_t>(limitRect.posX_ + edgeIntervalVp + i * (w + midIntervalVp));
            Rect curRect = { curX, y, w, h };
            WLOGFD("presetRectsForAllLevel: level %{public}u, id %{public}u, tileRect: [%{public}d %{public}d "
                "%{public}u %{public}u]", num, i, curX, y, w, h);
            curLevel.emplace_back(curRect);
        }
        presetRectsForAllLevel.emplace_back(curLevel);
    }
}

void WindowLayoutPolicyTile::InitTileQueue(DisplayId displayId)
{
    foregroundNodesMap_[displayId].clear();
    const auto& appWindowNodes = *(displayGroupWindowTree_[displayId][WindowRootNodeType::APP_WINDOW_NODE]);
    for (auto& node : appWindowNodes) {
        if (WindowHelper::IsMainWindow(node->GetWindowType())) {
            PushBackNodeInTileQueue(node, displayId);
        }
    }
}

bool WindowLayoutPolicyTile::IsTileRectSatisfiedWithSizeLimits(const sptr<WindowNode>& node)
{
    if (!WindowHelper::IsMainWindow(node->GetWindowType())) {
        return true;
    }
    const auto& displayId = node->GetDisplayId();
    auto& foregroundNodes = foregroundNodesMap_[displayId];
    auto num = foregroundNodes.size();
    if (num > maxTileWinNumMap_[displayId] || maxTileWinNumMap_[displayId] == 0) {
        return false;
    }

    // find if node already exits in foreground nodes map
    if (IsWindowAlreadyInTileQueue(node)) {
        return true;
    }

    UpdateWindowSizeLimits(node);
    const auto& presetRectsForAllLevel = presetRectsMap_[displayId];
    Rect tileRect;
    // if size of foreground nodes is equal to or more than max tile window number
    if (num == maxTileWinNumMap_[displayId]) {
        tileRect = *(presetRectsForAllLevel[num - 1].begin());
    } else {  // if size of foreground nodes is less than max tile window number
        tileRect = *(presetRectsForAllLevel[num].begin());
    }
    WLOGFD("id %{public}u, tileRect: [%{public}d %{public}d %{public}u %{public}u]",
        node->GetWindowId(), tileRect.posX_, tileRect.posY_, tileRect.width_, tileRect.height_);
    return WindowHelper::IsRectSatisfiedWithSizeLimits(tileRect, node->GetWindowUpdatedSizeLimits());
}

void WindowLayoutPolicyTile::PerformWindowLayout(const sptr<WindowNode>& node, WindowUpdateType updateType)
{
    HITRACE_METER(HITRACE_TAG_WINDOW_MANAGER);
    const auto& windowType = node->GetWindowType();
    const auto& requestRect = node->GetRequestRect();
    WLOGI("[PerformWindowLayout] windowId: %{public}u, windowType: %{public}u, updateType: %{public}u, requestRect: "
        "requestRect: [%{public}d, %{public}d, %{public}u, %{public}u]", node->GetWindowId(), windowType, updateType,
        requestRect.posX_, requestRect.posY_, requestRect.width_, requestRect.height_);
    FixWindowRectWithinDisplay(node);
    switch (updateType) {
        case WindowUpdateType::WINDOW_UPDATE_ADDED: {
            if (WindowHelper::IsMainWindow(windowType)) {
                PushBackNodeInTileQueue(node, node->GetDisplayId());
                LayoutTileQueue(node->GetDisplayId());
                return;
            }
            break;
        }
        case WindowUpdateType::WINDOW_UPDATE_REMOVED: {
            if (WindowHelper::IsMainWindow(windowType)) {
                RemoveNodeFromTileQueue(node);
                LayoutTileQueue(node->GetDisplayId());
            }
            NotifyClientAndAnimation(node, node->GetRequestRect(), WindowSizeChangeReason::HIDE);
            return;
        }
        default:
            WLOGFD("Update type is not add or remove");
    }
    LayoutWindowNode(node);
}

void WindowLayoutPolicyTile::LayoutTileQueue(DisplayId displayId)
{
    ApplyPresetRectForTileWindows(displayId);
    for (auto& node : foregroundNodesMap_[displayId]) {
        LayoutWindowNode(node);
    }
}

bool WindowLayoutPolicyTile::IsWindowAlreadyInTileQueue(const sptr<WindowNode>& node)
{
    auto& foregroundNodes = foregroundNodesMap_[node->GetDisplayId()];
    auto iter = std::find_if(foregroundNodes.begin(), foregroundNodes.end(),
                             [node](sptr<WindowNode> foregroundNode) {
                                 return foregroundNode->GetWindowId() == node->GetWindowId();
                             });
    if (iter != foregroundNodes.end()) {
        WLOGFD("Window is already in tile queue, windowId: %{public}d", node->GetWindowId());
        return true;
    }
    return false;
}

void WindowLayoutPolicyTile::PushBackNodeInTileQueue(const sptr<WindowNode>& node, DisplayId displayId)
{
    if (node == nullptr) {
        return;
    }
    if (IsWindowAlreadyInTileQueue(node)) {
        return;
    }

    if (!WindowHelper::IsWindowModeSupported(node->GetWindowModeSupportType(), WindowMode::WINDOW_MODE_FLOATING)) {
        WLOGFD("Window don't support floating mode that should be minimized, winId: %{public}u, "
            "windowModeSupportType: %{public}u", node->GetWindowId(), node->GetWindowModeSupportType());
        MinimizeApp::AddNeedMinimizeApp(node, MinimizeReason::INVALID_MODE_OR_SIZE_IN_TILE);
        return;
    }
    auto& foregroundNodes = foregroundNodesMap_[displayId];
    while (!foregroundNodes.empty() && foregroundNodes.size() >= maxTileWinNumMap_[displayId]) {
        auto removeNode = foregroundNodes.front();
        foregroundNodes.pop_front();
        WLOGFD("Minimize win in queue head for add new win, windowId: %{public}d", removeNode->GetWindowId());
        MinimizeApp::AddNeedMinimizeApp(removeNode, MinimizeReason::LAYOUT_TILE);
    }
    foregroundNodes.push_back(node);
    WLOGFD("Pusk back win in tile queue, displayId: %{public}" PRIu64", winId: %{public}d",
        displayId, node->GetWindowId());
}

void WindowLayoutPolicyTile::RemoveNodeFromTileQueue(const sptr<WindowNode>& node)
{
    if (node == nullptr) {
        return;
    }
    DisplayId displayId = node->GetDisplayId();
    auto& foregroundNodes = foregroundNodesMap_[displayId];
    auto iter = std::find(foregroundNodes.begin(), foregroundNodes.end(), node);
    if (iter != foregroundNodes.end()) {
        WLOGFD("Remove win in tile for win id: %{public}d", node->GetWindowId());
        foregroundNodes.erase(iter);
    }
}

bool WindowLayoutPolicyTile::IsValidTileQueueAndPresetRects(DisplayId displayId)
{
    auto& foregroundNodes = foregroundNodesMap_[displayId];
    uint32_t num = foregroundNodes.size();
    auto& presetRectsForAllLevel = presetRectsMap_[displayId];
    if (num > maxTileWinNumMap_[displayId] || num > presetRectsForAllLevel.size() || num == 0) {
        WLOGE("Invalid tile queue, foreground tileNum: %{public}u, maxTileNum: %{public}u, presetRectsForAllLevel: "
            "%{public}u", num, maxTileWinNumMap_[displayId], static_cast<uint32_t>(presetRectsForAllLevel.size()));
        return false;
    }
    auto& presetRect = presetRectsForAllLevel[num - 1];
    if (presetRect.size() != num) {
        WLOGE("Invalid preset rects, foreground tileNum: %{public}u, presetRect.size(): %{public}u",
            num, static_cast<uint32_t>(presetRect.size()));
        return false;
    }
    return true;
}

void WindowLayoutPolicyTile::RefreshTileQueue(DisplayId displayId,
    std::vector<sptr<WindowNode>>& needMinimizeNodes, std::vector<sptr<WindowNode>>& needRecoverNodes)
{
    /*
     * Usually, needMinimizeNodes and needRecoverNodes will be empty, there is no need to refresh tile queue
     */
    auto& foregroundNodes = foregroundNodesMap_[displayId];
    if (needMinimizeNodes.empty() && needRecoverNodes.empty()) {
        WLOGD("No need to refresh tileQueue");
        return;
    }

    WLOGD("Update tile queue for the nodes which should be minimized or recovered");
    for (auto& miniNode : needMinimizeNodes) {
        auto iter = std::find(foregroundNodes.begin(), foregroundNodes.end(), miniNode);
        if (iter != foregroundNodes.end()) {
            foregroundNodes.erase(iter);
        }
    }
    for (auto& recNode : needRecoverNodes) {
        auto iter = std::find(foregroundNodes.begin(), foregroundNodes.end(), recNode);
        if (iter == foregroundNodes.end()) {
            foregroundNodes.push_back(recNode);
        }
    }
    ApplyPresetRectForTileWindows(displayId);
    needMinimizeNodes.clear();
    needRecoverNodes.clear();
}

void WindowLayoutPolicyTile::ApplyPresetRectForTileWindows(DisplayId displayId)
{
    if (!(IsValidTileQueueAndPresetRects(displayId))) {
        return;
    }

    auto& foregroundNodes = foregroundNodesMap_[displayId];
    uint32_t num = foregroundNodes.size();
    auto rectIt = presetRectsMap_[displayId][num - 1].begin();
    std::vector<sptr<WindowNode>> needMinimizeNodes;
    std::vector<sptr<WindowNode>> needRecoverNodes;
    for (auto node : foregroundNodes) {
        auto& rect = (*rectIt);
        if (WindowHelper::IsRectSatisfiedWithSizeLimits(rect, node->GetWindowUpdatedSizeLimits())) {
            node->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
            // when change mode, need to reset shadow and radius
            WindowSystemEffect::SetWindowEffect(node);
            if (node->GetWindowToken()) {
                node->GetWindowToken()->UpdateWindowMode(WindowModeInfo{ WindowMode::WINDOW_MODE_FLOATING });
            }
            node->SetRequestRect(rect);
            node->SetDecoStatus(true);
            rectIt++;
            WLOGFD("Set preset rect for tileWin, id: %{public}d [%{public}d %{public}d %{public}d %{public}d]",
                node->GetWindowId(), rect.posX_, rect.posY_, rect.width_, rect.height_);
        } else {
            WLOGFD("Minimize node which can't be applied to tileRect, winId: %{public}u", node->GetWindowId());
            if (num == maxTileWinNumMap_[displayId]) {
                // if foreground nodes equal to max tileWinNum, means need to recover one node before minimize curNode
                auto recoverNode = MinimizeApp::GetRecoverdNodeFromMinimizeList();
                if (recoverNode != nullptr) {
                    needRecoverNodes.push_back(recoverNode);
                    WLOGFD("Cancel minimize node from minimizeList, winId: %{public}u", node->GetWindowId());
                }
            }
            needMinimizeNodes.push_back(node);
            MinimizeApp::AddNeedMinimizeApp(node, MinimizeReason::INVALID_MODE_OR_SIZE_IN_TILE);
            break;
        }
    }

    RefreshTileQueue(displayId, needMinimizeNodes, needRecoverNodes);
}

void WindowLayoutPolicyTile::UpdateLayoutRect(const sptr<WindowNode>& node)
{
    UpdateWindowSizeLimits(node);
    bool floatingWindow = (node->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING);
    bool needAvoid = (node->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID));
    Rect lastRect = node->GetWindowRect();
    Rect winRect = node->GetRequestRect();
    WLOGI("[Before TileLayout] windowId: %{public}u, mode: %{public}u, type: %{public}u requestRect: [%{public}d, "
        "%{public}d, %{public}u, %{public}u]", node->GetWindowId(), node->GetWindowMode(), node->GetWindowType(),
        winRect.posX_, winRect.posY_, winRect.width_, winRect.height_);

    if (!floatingWindow) { // fullscreen window
        const auto& displayRect = DisplayGroupInfo::GetInstance().GetDisplayRect(node->GetDisplayId());
        const auto& limitDisplayRect = limitRectMap_[node->GetDisplayId()];
        winRect = needAvoid ? limitDisplayRect : displayRect;
    }

    WLOGI("[After TileLayout] windowId: %{public}u, isDecor: %{public}u, winRect: [%{public}d, %{public}d, "
        "%{public}u, %{public}u]", node->GetWindowId(), node->GetDecoStatus(), winRect.posX_, winRect.posY_,
        winRect.width_, winRect.height_);
    node->SetWindowRect(winRect);

    // postProcess after update winRect
    CalcAndSetNodeHotZone(winRect, node);
    UpdateSurfaceBounds(node, winRect, lastRect);
    NotifyClientAndAnimation(node, winRect, node->GetWindowSizeChangeReason());
}
} // Rosen
} // OHOS
