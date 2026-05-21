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

#include <ability_manager_client.h>
#include <transaction/rs_transaction.h>
#include <unordered_set>

#include "display_manager_service_inner.h"
#include "dm_common.h"
#include "rs_adapter.h"
#include "singleton_container.h"
#include "window_adapter.h"
#include "window_group_mgr.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "window_manager_service.h"
#include "minimize_app.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {

namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowGroupMgr"};
}

WMError WindowGroupMgr::MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId)
{
    WLOGFD("%{public}s, topMissionId: %{public}d ", DumpVector(missionIds).c_str(), topMissionId);
    if (missionIds.empty()) {
        return WMError::WM_DO_NOTHING;
    }

    WMError res = WMError::WM_OK;
    for (auto it = missionIds.rbegin(); it != missionIds.rend(); it++) {
        if (*it == topMissionId) {
            continue;
        }
        WMError tempRes = MoveMissionToForeground(*it);
        res = tempRes != WMError::WM_OK ? tempRes : res;
    }

    if (topMissionId != DEFAULT_MISSION_ID) {
        WMError tempRes = MoveMissionToForeground(topMissionId);
        res = tempRes == WMError::WM_OK ? tempRes : res;
        WLOGFD("raise zOrder, missindId: %{public}d ", topMissionId);
        auto windowNode = windowRoot_->GetWindowNodeByMissionId(topMissionId);
        windowRoot_->RaiseZOrderForAppWindow(windowNode);
        RSTransactionAdapter::FlushImplicitTransaction(windowNode->GetRSUIContext());
    }
    return res;
}

WMError WindowGroupMgr::MoveMissionsToBackground(const std::vector<int32_t>& missionIds, std::vector<int32_t>& result)
{
    WLOGFD("%{public}s ", DumpVector(missionIds).c_str());
    if (missionIds.empty()) {
        return WMError::WM_DO_NOTHING;
    }

    std::vector<sptr<WindowNode>> windowNodes;
    std::vector<uint32_t> hideWindowIds;
    for (auto missionId : missionIds) {
        sptr<WindowNode> windowNode = windowRoot_->GetWindowNodeByMissionId(missionId);
        if (!windowNode) {
            continue;
        }
        windowNodes.emplace_back(windowNode);
    }
    std::sort(windowNodes.begin(), windowNodes.end(), [](const sptr<WindowNode>& w1, const sptr<WindowNode>& w2) {
        return w1->zOrder_ > w2->zOrder_;
    });

    std::unordered_set<DisplayId> displayIds;
    for (auto windowNode : windowNodes) {
        result.emplace_back(windowNode->abilityInfo_.missionId_);
        hideWindowIds.emplace_back(windowNode->GetWindowId());
        backupWindowModes_[windowNode->GetWindowId()] = windowNode->GetWindowMode();
        WLOGFD("windowId: %{public}d, missionId: %{public}d, node: %{public}s, zOrder: %{public}d, "
            "mode: %{public}d ", windowNode->GetWindowId(), windowNode->abilityInfo_.missionId_,
            (windowNode == nullptr ? "NUll" : windowNode->GetWindowName().c_str()),
            windowNode->zOrder_, windowNode->GetWindowMode());
        displayIds.insert(windowNode->GetDisplayId());
    }

    for (auto displayId : displayIds) {
        auto container = windowRoot_->GetOrCreateWindowNodeContainer(displayId);
        if (container != nullptr) {
            auto windowPair = container->GetDisplayGroupController()->GetWindowPairByDisplayId(displayId);
            if (windowPair && windowPair->GetDividerWindow()) {
                backupDividerWindowRect_[displayId] = windowPair->GetDividerWindow()->GetWindowRect();
            }
        }
    }

    WLOGFD("WindowGroupMgr::HideWindowGroup, hide WindowIds: %{public}s", DumpVector(hideWindowIds).c_str());
    windowRoot_->MinimizeTargetWindows(hideWindowIds);
    MinimizeApp::ExecuteMinimizeTargetReasons(MinimizeReason::GESTURE_ANIMATION);
    return WMError::WM_OK;
}

WMError WindowGroupMgr::MoveMissionToForeground(int32_t missionId)
{
    auto windowNode = windowRoot_->GetWindowNodeByMissionId(missionId);
    if (windowNode == nullptr || windowNode->GetWindowToken() == nullptr) {
        WLOGFE("GetWindowToken failed, missionId: %{public}d", missionId);
        return WMError::WM_ERROR_NULLPTR;
    }
    auto property = windowNode->GetWindowToken()->GetWindowProperty();
    if (property == nullptr) {
        WLOGFE("Get property failed , skip, missionId: %{public}d ", missionId);
        return WMError::WM_ERROR_NULLPTR;
    }
    std::set<DisplayId> displayIds;
    if (backupWindowModes_.count(windowNode->GetWindowId()) > 0) {
        auto mode = backupWindowModes_.at(windowNode->GetWindowId());
        if (WindowHelper::IsSplitWindowMode(mode)) {
            property->SetWindowMode(mode);
            windowNode->SetWindowMode(mode);
            // when change mode, need to reset shadow and radius
            WindowSystemEffect::SetWindowEffect(windowNode);
            displayIds.insert(windowNode->GetDisplayId());
            windowNode->GetWindowToken()->RestoreSplitWindowMode(static_cast<uint32_t>(mode));
            WLOGFD("Restore windowId: %{public}d, missionId: %{public}d, node: %{public}s, \
                zOrder: %{public}d, mode: %{public}d ",
                windowNode->GetWindowId(), windowNode->abilityInfo_.missionId_,
                (windowNode == nullptr ? "NUll" : windowNode->GetWindowName().c_str()),
                windowNode->zOrder_, windowNode->GetWindowMode());
        }
    }
    for (auto displayId : displayIds) {
        auto container = windowRoot_->GetOrCreateWindowNodeContainer(displayId);
        if (container != nullptr) {
            auto windowPair = container->GetDisplayGroupController()->GetWindowPairByDisplayId(displayId);
            if (windowPair != nullptr) {
                windowPair->SetAllSplitAppWindowsRestoring(true);
            }
        }
    }
    windowNode->GetWindowToken()->UpdateWindowState(WindowState::STATE_SHOWN);
    WindowManagerService::GetInstance().AddWindow(property);
    for (auto displayId : displayIds) {
        auto container = windowRoot_->GetOrCreateWindowNodeContainer(displayId);
        if (container != nullptr) {
            auto windowPair = container->GetDisplayGroupController()->GetWindowPairByDisplayId(displayId);
            if (windowPair != nullptr) {
                windowPair->SetAllSplitAppWindowsRestoring(false);
                container->GetLayoutPolicy()->SetSplitDividerWindowRects(backupDividerWindowRect_);
            }
        }
    }
    return WMError::WM_OK;
}

void WindowGroupMgr::OnWindowDestroyed(uint32_t windowId)
{
    WLOGFD("OnWindowDestroyed WindowIds: %{public}d", windowId);
    backupWindowModes_.erase(windowId);
}

void WindowGroupMgr::OnDisplayStateChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type)
{
    WLOGFD("OnDisplayStateChange displayId: %{public}" PRIu64", type: %{public}d", defaultDisplayId, type);
    if (type == DisplayStateChangeType::DESTROY) {
        backupDividerWindowRect_.erase(defaultDisplayId);
    }
}

}
}
