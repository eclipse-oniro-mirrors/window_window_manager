/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "fold_screen_controller/secondary_display_fold_policy.h"
#include "fold_screen_state_internel.h"
#include <hisysevent.h>
#include <hitrace_meter.h>
#include <transaction/rs_interfaces.h>
#include <parameters.h>
#include "rs_adapter.h"
#include "session/screen/include/screen_session.h"
#include "screen_session_manager.h"
#include "screen_scene_config.h"
#include "sensor_agent.h"
#include "sensor_agent_type.h"

#include "window_manager_hilog.h"

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

namespace OHOS::Rosen {
namespace {
const ScreenId SCREEN_ID_FULL = 0;
const ScreenId SCREEN_ID_MAIN = 5;
const int32_t REMOVE_DISPLAY_NODE = 0;
const int32_t ADD_DISPLAY_NODE = 1;
const int32_t MAIN_STATUS_WIDTH = 0;
const int32_t MAIN_STATUS_HEIGHT = 1;
const int32_t FULL_STATUS_WIDTH = 2;
const int32_t FULL_STATUS_HEIGHT = 3;
const int32_t GLOBAL_FULL_STATUS_WIDTH = 4;
const int32_t SCREEN_HEIGHT = 5;
const int32_t FULL_STATUS_OFFSET_X = 6;
const int32_t FULL_STATUS_OFFSET_Y = 7;
const int32_t GLOBAL_FULL_STATUS_OFFSET_Y = 8;
constexpr uint32_t HALF_DIVIDER = 2;
constexpr float MAIN_DISPLAY_ROTATION_DEGREE = -270;
constexpr float ROTATION_TRANSLATE_X = 612;
constexpr float ROTATION_TRANSLATE_Y = -612;
constexpr float FULL_NODE_POSITION_Z = 0.0f;
constexpr float MAIN_NODE_POSITION_Z = 1.0f;
constexpr int32_t FOLD_CREASE_RECT_SIZE = 8; // numbers of parameter on the current device is 8
const std::string g_FoldScreenRect = system::GetParameter("const.display.foldscreen.crease_region", "");
const std::string FOLD_CREASE_DELIMITER = ",;";
#ifdef TP_FEATURE_ENABLE
const int32_t TP_TYPE = 12;
const char* STATUS_MAIN = "version:3+main";
const char* STATUS_GLOBAL_FULL = "version:3+whole";
const char* STATUS_FULL = "version:3+minor";
#endif
static const std::unordered_set<FoldStatus> SUPPORTED_FOLD_STATUS_FOR_SECONDARY = {
    FoldStatus::EXPAND,
    FoldStatus::FOLDED,
    FoldStatus::HALF_FOLD,
    FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND,
    FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_HALF_FOLDED,
    FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_EXPAND,
    FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_HALF_FOLDED,
    FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_EXPAND,
    FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_HALF_FOLDED
};
} // namespace

SecondaryDisplayFoldPolicy::SecondaryDisplayFoldPolicy(std::recursive_mutex& displayInfoMutex,
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler)
    : displayInfoMutex_(displayInfoMutex), screenPowerTaskScheduler_(screenPowerTaskScheduler)
{
    InitScreenParams();
    std::vector<int32_t> screenParamsSigned;
    for (uint32_t ele : screenParams_) {
        screenParamsSigned.emplace_back(static_cast<int32_t>(ele));
    }
    ScreenId screenIdFull = 0;
    int32_t foldCreaseRegionABPosX = 0;
    int32_t foldCreaseRegionABPosY = screenParamsSigned[MAIN_STATUS_WIDTH]; // 1008
    int32_t foldCreaseRegionABPosHeight = screenParamsSigned[GLOBAL_FULL_STATUS_WIDTH] -
        screenParamsSigned[FULL_STATUS_WIDTH] - screenParamsSigned[MAIN_STATUS_WIDTH]; // 128
    int32_t foldCreaseRegionABPosWidth = screenParamsSigned[SCREEN_HEIGHT]; // 2232
    int32_t foldCreaseRegionBCPosX = 0;
    // 2224
    int32_t foldCreaseRegionBCPosY =
        screenParamsSigned[GLOBAL_FULL_STATUS_WIDTH] -
        ((screenParamsSigned[FULL_STATUS_WIDTH] / HALF_DIVIDER) - (foldCreaseRegionABPosHeight / HALF_DIVIDER));
    int32_t foldCreaseRegionBCPosWidth = screenParamsSigned[SCREEN_HEIGHT]; // 2232
    int32_t foldCreaseRegionBCPosHeight = foldCreaseRegionABPosHeight; // 128
    TLOGW(WmsLogTag::DMS, "created, screenIdFull = %{public}" PRIu64", foldCreaseRegionABPosX = %{public}d,\
        foldCreaseRegionABPosY = %{public}d, foldCreaseRegionABPosHeight = %{public}d,\
        foldCreaseRegionABPosWidth = %{public}d, foldCreaseRegionBCPosX = %{public}d,\
        foldCreaseRegionBCPosY = %{public}d, foldCreaseRegionBCPosWidth = %{public}d,\
        foldCreaseRegionBCPosHeight = %{public}d", screenIdFull, foldCreaseRegionABPosX, foldCreaseRegionABPosY,
        foldCreaseRegionABPosHeight, foldCreaseRegionABPosWidth, foldCreaseRegionBCPosX, foldCreaseRegionBCPosY,
        foldCreaseRegionBCPosWidth, foldCreaseRegionBCPosHeight);
    std::vector<DMRect> rect = {
        {
            foldCreaseRegionABPosX, foldCreaseRegionABPosY,
            static_cast<uint32_t>(foldCreaseRegionABPosWidth), static_cast<uint32_t>(foldCreaseRegionABPosHeight)
        },
        {
            foldCreaseRegionBCPosX, foldCreaseRegionBCPosY,
            static_cast<uint32_t>(foldCreaseRegionBCPosWidth), static_cast<uint32_t>(foldCreaseRegionBCPosHeight)
        }
    };
    currentFoldCreaseRegion_ = new FoldCreaseRegion(screenIdFull, rect);
}

void SecondaryDisplayFoldPolicy::GetAllCreaseRegion(std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems) const
{
    FoldCreaseRegionItem MLandCreaseItem{DisplayOrientation::LANDSCAPE, FoldDisplayMode::MAIN,
        FoldCreaseRegion(0, {})};
    FoldCreaseRegionItem MPorCreaseItem{DisplayOrientation::PORTRAIT, FoldDisplayMode::MAIN,
        FoldCreaseRegion(0, {})};
    FoldCreaseRegionItem FLandCreaseItem{DisplayOrientation::LANDSCAPE, FoldDisplayMode::FULL,
        GetStatusFullFoldCreaseRegion(false)};
    FoldCreaseRegionItem FPorCreaseItem{DisplayOrientation::PORTRAIT, FoldDisplayMode::FULL,
        GetStatusFullFoldCreaseRegion(true)};
    FoldCreaseRegionItem GLandCreaseItem{DisplayOrientation::LANDSCAPE, FoldDisplayMode::GLOBAL_FULL,
        GetStatusGlobalFullFoldCreaseRegion(false)};
    FoldCreaseRegionItem GPorCreaseItem{DisplayOrientation::PORTRAIT, FoldDisplayMode::GLOBAL_FULL,
        GetStatusGlobalFullFoldCreaseRegion(true)};
    foldCreaseRegionItems.push_back(MLandCreaseItem);
    foldCreaseRegionItems.push_back(MPorCreaseItem);
    foldCreaseRegionItems.push_back(FLandCreaseItem);
    foldCreaseRegionItems.push_back(FPorCreaseItem);
    foldCreaseRegionItems.push_back(GLandCreaseItem);
    foldCreaseRegionItems.push_back(GPorCreaseItem);
}

FoldCreaseRegion SecondaryDisplayFoldPolicy::GetStatusFullFoldCreaseRegion(bool isVertical) const
{
    std::vector<int32_t> foldRect = FoldScreenStateInternel::StringFoldRectSplitToInt(g_FoldScreenRect,
        FOLD_CREASE_DELIMITER);
    if (foldRect.size() != FOLD_CREASE_RECT_SIZE) {
        TLOGE(WmsLogTag::DMS, "foldRect is invalid");
        return FoldCreaseRegion(0, {});
    }

    ScreenId screenIdFull = 0;
    std::vector<DMRect> foldCreaseRect;
    GetStatusFullFoldCreaseRect(isVertical, foldRect, foldCreaseRect);
    return FoldCreaseRegion(screenIdFull, foldCreaseRect);
}

void SecondaryDisplayFoldPolicy::GetStatusFullFoldCreaseRect(bool isVertical,
    const std::vector<int32_t>& foldRect, std::vector<DMRect>& foldCreaseRect) const
{
    int32_t liveCreaseRegionPosX; // live Crease Region PosX
    int32_t liveCreaseRegionPosY; // live Crease Region PosY
    uint32_t liveCreaseRegionPosWidth; // live Crease Region PosWidth
    uint32_t liveCreaseRegionPosHeight; // live Crease Region PosHeight
    if (isVertical) {
        TLOGI(WmsLogTag::DMS, "the current FoldCreaseRect is vertical");
        liveCreaseRegionPosX = foldRect[0];
        liveCreaseRegionPosY = foldRect[1];
        liveCreaseRegionPosWidth = static_cast<uint32_t>(foldRect[2]);
        liveCreaseRegionPosHeight = static_cast<uint32_t>(foldRect[3]);
    } else {
        TLOGI(WmsLogTag::DMS, "the current FoldCreaseRect is horizontal");
        liveCreaseRegionPosX = foldRect[1];
        liveCreaseRegionPosY = foldRect[0];
        liveCreaseRegionPosWidth = static_cast<uint32_t>(foldRect[3]);
        liveCreaseRegionPosHeight = static_cast<uint32_t>(foldRect[2]);
    }
    foldCreaseRect = {
        {
            liveCreaseRegionPosX, liveCreaseRegionPosY,
            liveCreaseRegionPosWidth, liveCreaseRegionPosHeight
        }
    };
    return;
}

FoldCreaseRegion SecondaryDisplayFoldPolicy::GetStatusGlobalFullFoldCreaseRegion(bool isVertical) const
{
    std::vector<int32_t> foldRect = FoldScreenStateInternel::StringFoldRectSplitToInt(g_FoldScreenRect,
        FOLD_CREASE_DELIMITER);
    if (foldRect.size() != FOLD_CREASE_RECT_SIZE) {
        TLOGE(WmsLogTag::DMS, "foldRect is invalid");
        return FoldCreaseRegion(0, {});
    }

    ScreenId screenIdFull = 0;
    std::vector<DMRect> foldCreaseRect;
    GetStatusGlobalFullFoldCreaseRect(isVertical, foldRect, foldCreaseRect);
    return FoldCreaseRegion(screenIdFull, foldCreaseRect);
}

void SecondaryDisplayFoldPolicy::GetStatusGlobalFullFoldCreaseRect(bool isVertical,
    const std::vector<int32_t>& foldRect, std::vector<DMRect>& foldCreaseRect) const
{
    int32_t liveCreaseRegionABPosX; // live Crease Region ABPosX
    int32_t liveCreaseRegionABPosY; // live Crease Region ABPosY
    uint32_t liveCreaseRegionABPosWidth; // live Crease Region ABPosWidth
    uint32_t liveCreaseRegionABPosHeight; // live Crease Region ABPosHeight
    int32_t liveCreaseRegionBCPosX; // live Crease Region BCPosX
    int32_t liveCreaseRegionBCPosY; // live Crease Region BCPosY
    uint32_t liveCreaseRegionBCPosWidth; // live Crease Region BCPosWidth
    uint32_t liveCreaseRegionBCPosHeight; // live Crease Region BCPosHeight
    if (isVertical) {
        TLOGI(WmsLogTag::DMS, "the current FoldCreaseRect is vertical");
        liveCreaseRegionABPosX = foldRect[0];
        liveCreaseRegionABPosY = foldRect[1];
        liveCreaseRegionABPosWidth = static_cast<uint32_t>(foldRect[2]);
        liveCreaseRegionABPosHeight = static_cast<uint32_t>(foldRect[3]);
        liveCreaseRegionBCPosX = foldRect[4];
        liveCreaseRegionBCPosY = foldRect[5];
        liveCreaseRegionBCPosWidth = static_cast<uint32_t>(foldRect[6]);
        liveCreaseRegionBCPosHeight = static_cast<uint32_t>(foldRect[7]);
    } else {
        TLOGI(WmsLogTag::DMS, "the current FoldCreaseRect is horizontal");
        liveCreaseRegionABPosX = foldRect[1];
        liveCreaseRegionABPosY = foldRect[0];
        liveCreaseRegionABPosWidth = static_cast<uint32_t>(foldRect[3]);
        liveCreaseRegionABPosHeight = static_cast<uint32_t>(foldRect[2]);
        liveCreaseRegionBCPosX = foldRect[5];
        liveCreaseRegionBCPosY = foldRect[4];
        liveCreaseRegionBCPosWidth = static_cast<uint32_t>(foldRect[7]);
        liveCreaseRegionBCPosHeight = static_cast<uint32_t>(foldRect[6]);
    }
    foldCreaseRect = {
        {
            liveCreaseRegionABPosX, liveCreaseRegionABPosY,
            static_cast<uint32_t>(liveCreaseRegionABPosWidth), static_cast<uint32_t>(liveCreaseRegionABPosHeight)
        },
        {
            liveCreaseRegionBCPosX, liveCreaseRegionBCPosY,
            static_cast<uint32_t>(liveCreaseRegionBCPosWidth), static_cast<uint32_t>(liveCreaseRegionBCPosHeight)
        }
    };
    return;
}

void SecondaryDisplayFoldPolicy::ChangeScreenDisplayMode(FoldDisplayMode displayMode, DisplayModeChangeReason reason)
{
    if (GetPhysicalFoldLockFlag() && reason != DisplayModeChangeReason::FORCE_SET) {
        TLOGI(WmsLogTag::DMS, "Fold status is locked, can't change to display mode: %{public}d", displayMode);
        return;
    }
    SetLastCacheDisplayMode(displayMode);
    if (GetModeChangeRunningStatus()) {
        TLOGW(WmsLogTag::DMS, "last process not complete, skip mode: %{public}d", displayMode);
        return;
    }
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_FULL);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "default screenSession is null");
        return;
    }
    {
        std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
        lastDisplayMode_ = displayMode;
    }
    if (displayMode == FoldDisplayMode::UNKNOWN) {
        TLOGW(WmsLogTag::DMS, "displayMode is unknown");
    } else {
        ChangeSuperScreenDisplayMode(screenSession, displayMode);
    }
    ReportFoldDisplayModeChange(displayMode);
    {
        std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
        currentDisplayMode_ = displayMode;
    }
    if (displayMode == FoldDisplayMode::GLOBAL_FULL) {
        TLOGW(WmsLogTag::DMS, "Set device status to STATUS_GLOBAL_FULL");
        SetDeviceStatus(static_cast<uint32_t>(DMDeviceStatus::STATUS_GLOBAL_FULL));
        system::SetParameter("persist.dms.device.status",
            std::to_string(static_cast<uint32_t>(DMDeviceStatus::STATUS_GLOBAL_FULL)));
    } else {
        TLOGW(WmsLogTag::DMS, "Set device status to UNKNOWN");
        SetDeviceStatus(static_cast<uint32_t>(DMDeviceStatus::UNKNOWN));
        system::SetParameter("persist.dms.device.status",
            std::to_string(static_cast<uint32_t>(DMDeviceStatus::UNKNOWN)));
    }
    ScreenSessionManager::GetInstance().NotifyDisplayModeChanged(displayMode);
    TLOGD(WmsLogTag::DMS, "End change display mode: %{public}d, reason: %{public}d", displayMode, reason);
    return;
}

void SecondaryDisplayFoldPolicy::SendSensorResult(FoldStatus foldStatus)
{
    TLOGW(WmsLogTag::DMS, "FoldStatus: %{public}d", foldStatus);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    ChangeScreenDisplayMode(displayMode);
}

void SecondaryDisplayFoldPolicy::CloseCoordinationScreen()
{
    std::lock_guard<std::mutex> lock(coordinationMutex_);
    if (!ScreenSessionManager::GetInstance().GetCoordinationFlag()) {
        TLOGW(WmsLogTag::DMS, "CloseCoordinationScreen skipped, current coordination flag is false");
        return;
    }
    TLOGI(WmsLogTag::DMS, "Close Coordination Screen current mode=%{public}d", currentDisplayMode_);
    ScreenSessionManager::GetInstance().NotifyRSCoordination(false);
    AddOrRemoveDisplayNodeToTree(SCREEN_ID_MAIN, REMOVE_DISPLAY_NODE);

    ScreenSessionManager::GetInstance().OnScreenChange(SCREEN_ID_MAIN, ScreenEvent::DISCONNECTED);
    ScreenSessionManager::GetInstance().SetCoordinationFlag(false);
}

void SecondaryDisplayFoldPolicy::ChangeScreenDisplayModeToCoordination()
{
    std::lock_guard<std::mutex> lock(coordinationMutex_);
    if (ScreenSessionManager::GetInstance().GetCoordinationFlag()) {
        TLOGW(WmsLogTag::DMS, "change displaymode to coordination skipped, current coordination flag is true");
        return;
    }
    TLOGI(WmsLogTag::DMS, "change displaymode to coordination current mode=%{public}d", currentDisplayMode_);
    ScreenSessionManager::GetInstance().NotifyRSCoordination(true);
    ScreenSessionManager::GetInstance().SetCoordinationFlag(true);
    ScreenSessionManager::GetInstance().OnScreenChange(SCREEN_ID_MAIN, ScreenEvent::CONNECTED);

    // set position_Z for dual displayNode
    UpdatePositionZForDualDisplayNode();
    AddOrRemoveDisplayNodeToTree(SCREEN_ID_MAIN, ADD_DISPLAY_NODE);
}

void SecondaryDisplayFoldPolicy::AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command)
{
    TLOGI(WmsLogTag::DMS, "AddOrRemoveDisplayNodeToTree, screenId: %{public}" PRIu64 ", command: %{public}d",
        screenId, command);
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "AddOrRemoveDisplayNodeToTree, screenSession is null");
        return;
    }
    std::shared_ptr<RSDisplayNode> displayNode = screenSession->GetDisplayNode();
    if (displayNode == nullptr) {
        TLOGE(WmsLogTag::DMS, "AddOrRemoveDisplayNodeToTree, displayNode is null");
        return;
    }

    if (command == ADD_DISPLAY_NODE) {
        UpdateDisplayNodeBasedOnScreenId(screenId, displayNode);
        displayNode->AddDisplayNodeToTree();
    } else if (command == REMOVE_DISPLAY_NODE) {
        displayNode->RemoveDisplayNodeFromTree();
    }
    displayNode = nullptr;
    TLOGI(WmsLogTag::DMS, "add or remove displayNode");
    RSTransactionAdapter::FlushImplicitTransaction(screenSession->GetRSUIContext());
}

const std::unordered_set<FoldStatus>& SecondaryDisplayFoldPolicy::GetSupportedFoldStatus() const
{
    return SUPPORTED_FOLD_STATUS_FOR_SECONDARY;
}

void SecondaryDisplayFoldPolicy::UpdateDisplayNodeBasedOnScreenId(ScreenId screenId,
    std::shared_ptr<RSDisplayNode> displayNode)
{
    if (screenId != SCREEN_ID_MAIN) {
        return;
    }
    TLOGW(WmsLogTag::DMS, "UpdateDisplayNodeBasedOnScreenId: %{public}" PRIu64, screenId);
    displayNode->SetScreenId(SCREEN_ID_FULL);
    displayNode->SetRotation(MAIN_DISPLAY_ROTATION_DEGREE);
    displayNode->SetTranslate({ROTATION_TRANSLATE_X, ROTATION_TRANSLATE_Y});
    displayNode->SetFrame(0, 0, screenParams_[MAIN_STATUS_WIDTH], screenParams_[SCREEN_HEIGHT]);
    displayNode->SetBounds(0, 0, screenParams_[MAIN_STATUS_WIDTH], screenParams_[SCREEN_HEIGHT]);
}

void SecondaryDisplayFoldPolicy::InitPositionZInfos()
{
    if (!dualDisplayNodePositionZ_.empty()) {
        return;
    }
    dualDisplayNodePositionZ_.insert(std::make_pair(SCREEN_ID_FULL, FULL_NODE_POSITION_Z));
    dualDisplayNodePositionZ_.insert(std::make_pair(SCREEN_ID_MAIN, MAIN_NODE_POSITION_Z));
}

void SecondaryDisplayFoldPolicy::UpdatePositionZForDualDisplayNode()
{
    InitPositionZInfos();
    for (const auto& pair : dualDisplayNodePositionZ_) {
        int screenId = pair.first;
        float positionZ = pair.second;
        sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "AddOrRemoveDisplayNodeToTree, screenSession is null");
            continue;
        }
        std::shared_ptr<RSDisplayNode> displayNode = screenSession->GetDisplayNode();
        if (displayNode == nullptr) {
            TLOGE(WmsLogTag::DMS, "AddOrRemoveDisplayNodeToTree, displayNode is null");
            continue;
        }
        displayNode->SetPositionZ(positionZ);
        RSTransactionAdapter::FlushImplicitTransaction(screenSession->GetRSUIContext());
    }
}

void SecondaryDisplayFoldPolicy::ExitCoordination()
{
    std::lock_guard<std::mutex> lock(coordinationMutex_);
    if (!ScreenSessionManager::GetInstance().GetCoordinationFlag()) {
        TLOGW(WmsLogTag::DMS, "ExitCoordination skipped, current coordination flag is false");
        return;
    }
    ScreenSessionManager::GetInstance().NotifyRSCoordination(false);
    ScreenSessionManager::GetInstance().SetKeyguardDrawnDoneFlag(false);
    AddOrRemoveDisplayNodeToTree(SCREEN_ID_MAIN, REMOVE_DISPLAY_NODE);
    ScreenSessionManager::GetInstance().OnScreenChange(SCREEN_ID_MAIN, ScreenEvent::DISCONNECTED);
    ScreenSessionManager::GetInstance().SetCoordinationFlag(false);
#ifdef TP_FEATURE_ENABLE
    TLOGI(WmsLogTag::DMS, "ExitCoordination SetTpFeatureConfig to full");
    RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE, STATUS_FULL, TpFeatureConfigType::AFT_TP_FEATURE);
#endif
    FoldDisplayMode displayMode = GetModeMatchStatus();
    currentDisplayMode_ = displayMode;
    lastDisplayMode_ = displayMode;
    TLOGI(WmsLogTag::DMS, "Exit coordination, current display mode:%{public}d", displayMode);
    ScreenSessionManager::GetInstance().NotifyDisplayModeChanged(displayMode);
}

void SecondaryDisplayFoldPolicy::SetOnBootAnimation(bool onBootAnimation)
{
    TLOGW(WmsLogTag::DMS, "onBootAnimation: %{public}d", onBootAnimation);
    onBootAnimation_ = onBootAnimation;
    if (!onBootAnimation_) {
        TLOGW(WmsLogTag::DMS, "when boot animation finished, change display mode");
        isChangeScreenWhenBootCompleted = true;
        RecoverWhenBootAnimationExit();
    }
}

void SecondaryDisplayFoldPolicy::RecoverWhenBootAnimationExit()
{
    TLOGW(WmsLogTag::DMS, "CurrentScreen(%{public}" PRIu64 ")", screenId_);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    ChangeScreenDisplayMode(displayMode);
}

FoldDisplayMode SecondaryDisplayFoldPolicy::GetModeMatchStatus()
{
    return GetModeMatchStatus(currentFoldStatus_);
}

FoldDisplayMode SecondaryDisplayFoldPolicy::GetModeMatchStatus(FoldStatus targetFoldStatus)
{
    FoldDisplayMode displayMode = FoldDisplayMode::MAIN;
    switch (targetFoldStatus) {
        case FoldStatus::EXPAND: {
            displayMode = FoldDisplayMode::FULL;
            break;
        }
        case FoldStatus::FOLDED: {
            displayMode = FoldDisplayMode::MAIN;
            break;
        }
        case FoldStatus::HALF_FOLD: {
            displayMode = FoldDisplayMode::FULL;
            break;
        }
        case FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND: {
            displayMode = FoldDisplayMode::GLOBAL_FULL;
            break;
        }
        case FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_HALF_FOLDED: {
            displayMode = FoldDisplayMode::GLOBAL_FULL;
            break;
        }
        case FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_EXPAND: {
            displayMode = FoldDisplayMode::MAIN;
            break;
        }
        case FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_HALF_FOLDED: {
            displayMode = FoldDisplayMode::MAIN;
            break;
        }
        case FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_EXPAND: {
            displayMode = FoldDisplayMode::GLOBAL_FULL;
            break;
        }
        case FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_HALF_FOLDED: {
            displayMode = FoldDisplayMode::GLOBAL_FULL;
            break;
        }
        default: {
            TLOGW(WmsLogTag::DMS, "FoldStatus is invalid");
        }
    }
    TLOGW(WmsLogTag::DMS, "displayMode: %{public}d", displayMode);
    return displayMode;
}

void SecondaryDisplayFoldPolicy::ChangeSuperScreenDisplayMode(sptr<ScreenSession> screenSession,
    FoldDisplayMode displayMode)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:ChangeScreenDisplayMode(displayMode = %" PRIu64")", displayMode);
    {
        std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
        if (currentDisplayMode_ == displayMode && !isChangeScreenWhenBootCompleted) {
            TLOGW(WmsLogTag::DMS, "already in displayMode %{public}d", displayMode);
            return;
        }
        if (isChangeScreenWhenBootCompleted) {
            isChangeScreenWhenBootCompleted = false;
        }
    }
    SetSecondaryDisplayModeChangeStatus(true);
    SendPropertyChangeResult(screenSession, SCREEN_ID_FULL, ScreenPropertyChangeReason::FOLD_SCREEN_EXPAND,
        displayMode);
    if (currentDisplayMode_ != displayMode) {
        if ((currentDisplayMode_ == FoldDisplayMode::GLOBAL_FULL && displayMode == FoldDisplayMode::FULL) ||
            (currentDisplayMode_ == FoldDisplayMode::FULL && displayMode == FoldDisplayMode::MAIN) ||
            (currentDisplayMode_ == FoldDisplayMode::GLOBAL_FULL && displayMode == FoldDisplayMode::MAIN)) {
                return;
        }
    }
    auto taskScreenOnFullOn = [=] {
        screenId_ = SCREEN_ID_FULL;
        PowerMgr::PowerMgrClient::GetInstance().WakeupDeviceAsync();
        SetdisplayModeChangeStatus(false);
    };
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOnFullOn, "screenOnFullOnTask");
}

void SecondaryDisplayFoldPolicy::SendPropertyChangeResult(sptr<ScreenSession> screenSession, ScreenId screenId,
    ScreenPropertyChangeReason reason, FoldDisplayMode displayMode)
{
    std::lock_guard<std::recursive_mutex> lock_info(displayInfoMutex_);
    screenProperty_ = ScreenSessionManager::GetInstance().GetPhyScreenProperty(screenId);
    screenSession->SetPhyScreenId(screenId);
    bool isNeedNotifyFoldProperty = true;
    bool isNeedToSetSwitch = true;
    switch (displayMode) {
        case FoldDisplayMode::FULL: {
            if (currentDisplayMode_ == FoldDisplayMode::COORDINATION) {
                if (currentFoldStatus_ == FoldStatus::EXPAND) {
                    isNeedToSetSwitch = false;
                    isNeedNotifyFoldProperty = false;
                }
                CloseCoordinationScreen();
            }
            SetStatusFullActiveRectAndTpFeature(screenSession, screenProperty_, isNeedToSetSwitch);
            break;
        }
        case FoldDisplayMode::MAIN: {
            if (currentDisplayMode_ == FoldDisplayMode::COORDINATION) {
                CloseCoordinationScreen();
            }
            reason = ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING;
            SetStatusMainActiveRectAndTpFeature(screenSession, screenProperty_);
            break;
        }
        case FoldDisplayMode::GLOBAL_FULL: {
            if (currentDisplayMode_ == FoldDisplayMode::COORDINATION) {
                CloseCoordinationScreen();
            }
            SetStatusGlobalFullActiveRectAndTpFeature(screenSession, screenProperty_);
            break;
        }
        case FoldDisplayMode::COORDINATION: {
            if (currentDisplayMode_ == FoldDisplayMode::FULL) {
                isNeedNotifyFoldProperty = false;
                ChangeScreenDisplayModeToCoordination();
                SetStatusConditionalActiveRectAndTpFeature(screenProperty_);
            }
            break;
        }
        default: {
            TLOGW(WmsLogTag::DMS, "unKnown displayMode");
            break;
        }
    }
    HandlePropertyChange(screenSession, screenProperty_, reason, displayMode, isNeedNotifyFoldProperty);
}

void SecondaryDisplayFoldPolicy::HandlePropertyChange(sptr<ScreenSession> screenSession, ScreenProperty& ScreenProperty,
    ScreenPropertyChangeReason reason, FoldDisplayMode displayMode, bool isNeedNotifyFoldProperty)
{
    if (!ScreenSessionManager::GetInstance().GetClientProxy()) {
        bool firstSCBConnect = ScreenSessionManager::GetInstance().GetFirstSCBConnect();
        screenSession->UpdatePropertyByFoldControl(ScreenProperty, displayMode, firstSCBConnect);

        if (isNeedNotifyFoldProperty) {
            screenSession->PropertyChange(screenSession->GetScreenProperty(), reason);
        } else {
            TLOGI(WmsLogTag::DMS, "PropertyChange...set displayModeChangeStatus");
            SetSecondaryDisplayModeChangeStatus(false);
        }

        if (displayMode == FoldDisplayMode::MAIN) {
            screenSession->SetRotationAndScreenRotationOnly(Rotation::ROTATION_0);
        }

        TLOGI(WmsLogTag::DMS, "screenBounds : width_= %{public}f, height_= %{public}f",
            screenSession->GetScreenProperty().GetBounds().rect_.width_,
            screenSession->GetScreenProperty().GetBounds().rect_.height_);
    } else {
        screenSession->NotifyFoldPropertyChange(ScreenProperty, reason, displayMode);
    }
}

void SecondaryDisplayFoldPolicy::SetStatusFullActiveRectAndTpFeature(const sptr<ScreenSession>& screenSession,
    ScreenProperty &screenProperty, bool isNeedToSetSwitch)
{
    if (screenParams_.size() < FULL_STATUS_OFFSET_X + 1) {
        return;
    }
    auto fullStatusScreenBounds = RRect({0, screenParams_[FULL_STATUS_OFFSET_X],
        screenParams_[FULL_STATUS_WIDTH], screenParams_[FULL_STATUS_HEIGHT]}, 0.0f, 0.0f);
    screenProperty.SetBounds(fullStatusScreenBounds);
    screenSession->SetScreenAreaOffsetY(screenParams_[FULL_STATUS_OFFSET_X]);
    screenSession->SetScreenAreaHeight(screenParams_[FULL_STATUS_WIDTH]);
    screenSession->SetScreenAreaWidth(screenParams_[FULL_STATUS_HEIGHT]);
    screenProperty.SetInputOffset(static_cast<int32_t>(screenParams_[FULL_STATUS_OFFSET_X]),
        static_cast<int32_t>(screenParams_[FULL_STATUS_OFFSET_Y]));
    OHOS::Rect rectCur{
        .x = 0,
        .y = screenParams_[FULL_STATUS_OFFSET_X],
        .w = screenParams_[FULL_STATUS_HEIGHT],
        .h = screenParams_[FULL_STATUS_WIDTH],
    };
    if (!onBootAnimation_) {
        if (isNeedToSetSwitch) {
            RSInterfaces::GetInstance().NotifyScreenSwitched();
        }
        auto response = RSInterfaces::GetInstance().SetScreenActiveRect(0, rectCur);
        TLOGI(WmsLogTag::DMS, "rs response is %{public}ld", static_cast<long>(response));
    }
#ifdef TP_FEATURE_ENABLE
    RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE, STATUS_FULL, TpFeatureConfigType::AFT_TP_FEATURE);
#endif
    SetSecondaryDisplayModeChangeStatus(false);
}

void SecondaryDisplayFoldPolicy::SetStatusMainActiveRectAndTpFeature(const sptr<ScreenSession>& screenSession,
    ScreenProperty &screenProperty)
{
    if (screenParams_.size() < SCREEN_HEIGHT + 1) {
        return;
    }
    auto mianStatusScreenBounds =
        RRect({0, 0, screenParams_[MAIN_STATUS_WIDTH], screenParams_[MAIN_STATUS_HEIGHT]}, 0.0f, 0.0f);
    screenProperty.SetBounds(mianStatusScreenBounds);
    screenSession->SetScreenAreaOffsetY(0);
    screenSession->SetScreenAreaHeight(screenParams_[MAIN_STATUS_WIDTH]);
    screenSession->SetScreenAreaHeight(screenParams_[MAIN_STATUS_HEIGHT]);
    OHOS::Rect rectCur{
        .x = 0,
        .y = 0,
        .w = screenParams_[MAIN_STATUS_HEIGHT],
        .h = screenParams_[MAIN_STATUS_WIDTH],
    };
    if (!onBootAnimation_) {
        RSInterfaces::GetInstance().NotifyScreenSwitched();
        auto response = RSInterfaces::GetInstance().SetScreenActiveRect(0, rectCur);
        TLOGI(WmsLogTag::DMS, "rs response is %{public}ld", static_cast<long>(response));
    }
#ifdef TP_FEATURE_ENABLE
    RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE, STATUS_MAIN, TpFeatureConfigType::AFT_TP_FEATURE);
#endif
    SetSecondaryDisplayModeChangeStatus(false);
}

void SecondaryDisplayFoldPolicy::SetStatusGlobalFullActiveRectAndTpFeature(const sptr<ScreenSession>& screenSession,
    ScreenProperty &screenProperty)
{
    if (screenParams_.size() < SCREEN_HEIGHT + 1) {
        return;
    }
    auto globalFullStatusScreenBounds =
        RRect({0, 0, screenParams_[GLOBAL_FULL_STATUS_WIDTH], screenParams_[SCREEN_HEIGHT]}, 0.0f, 0.0f);
    screenProperty.SetBounds(globalFullStatusScreenBounds);
    screenSession->SetScreenAreaOffsetY(0);
    screenSession->SetScreenAreaHeight(screenParams_[GLOBAL_FULL_STATUS_WIDTH]);
    screenSession->SetScreenAreaWidth(screenParams_[SCREEN_HEIGHT]);
    screenProperty.SetInputOffset(0, static_cast<int32_t>(screenParams_[GLOBAL_FULL_STATUS_OFFSET_Y]));
    OHOS::Rect rectCur{
        .x = 0,
        .y = 0,
        .w = screenParams_[SCREEN_HEIGHT],
        .h = screenParams_[GLOBAL_FULL_STATUS_WIDTH],
    };
    if (!onBootAnimation_) {
        RSInterfaces::GetInstance().NotifyScreenSwitched();
        auto response = RSInterfaces::GetInstance().SetScreenActiveRect(0, rectCur);
        TLOGI(WmsLogTag::DMS, "rs response is %{public}ld", static_cast<long>(response));
    }
#ifdef TP_FEATURE_ENABLE
    RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE, STATUS_GLOBAL_FULL, TpFeatureConfigType::AFT_TP_FEATURE);
#endif
    SetSecondaryDisplayModeChangeStatus(false);
}

void SecondaryDisplayFoldPolicy::SetStatusConditionalActiveRectAndTpFeature(ScreenProperty &screenProperty)
{
    auto globalFullStatusScreenBounds = RRect({0, screenParams_[FULL_STATUS_OFFSET_X],
        screenParams_[FULL_STATUS_WIDTH], screenParams_[SCREEN_HEIGHT]}, 0.0f, 0.0f);
    screenProperty.SetBounds(globalFullStatusScreenBounds);
    screenProperty.SetScreenAreaOffsetY(screenParams_[FULL_STATUS_OFFSET_X]);
    screenProperty.SetScreenAreaHeight(screenParams_[FULL_STATUS_WIDTH]);
    screenProperty.SetInputOffset(static_cast<int32_t>(screenParams_[FULL_STATUS_OFFSET_X]), 0);
    OHOS::Rect rectCur {
        .x = 0,
        .y = 0,
        .w = screenParams_[SCREEN_HEIGHT],
        .h = screenParams_[GLOBAL_FULL_STATUS_WIDTH],
    };
    if (!onBootAnimation_) {
        RSInterfaces::GetInstance().NotifyScreenSwitched();
        auto response = RSInterfaces::GetInstance().SetScreenActiveRect(0, rectCur);
        TLOGI(WmsLogTag::DMS, "rs response is %{public}d", static_cast<uint32_t>(response));
    }
#ifdef TP_FEATURE_ENABLE
    RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE, STATUS_FULL, TpFeatureConfigType::AFT_TP_FEATURE);
#endif
    SetSecondaryDisplayModeChangeStatus(false);
}

void SecondaryDisplayFoldPolicy::ReportFoldDisplayModeChange(FoldDisplayMode displayMode)
{
    int32_t mode = static_cast<int32_t>(displayMode);
    TLOGW(WmsLogTag::DMS, "displayMode: %{public}d", mode);
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "DISPLAY_MODE",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "FOLD_DISPLAY_MODE", mode);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "write HiSysEvent error, ret: %{public}d", ret);
    }
}

sptr<FoldCreaseRegion> SecondaryDisplayFoldPolicy::GetCurrentFoldCreaseRegion()
{
    return currentFoldCreaseRegion_;
}

FoldCreaseRegion SecondaryDisplayFoldPolicy::GetLiveCreaseRegion()
{
    TLOGI(WmsLogTag::DMS, "enter");
    std::lock_guard<std::mutex> lock_mode(liveCreaseRegionMutex_);
    FoldDisplayMode displayMode = GetScreenDisplayMode();
    if (displayMode == FoldDisplayMode::UNKNOWN || displayMode == FoldDisplayMode::MAIN) {
        return FoldCreaseRegion(0, {});
    }
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_FULL);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "default screenSession is null");
        return FoldCreaseRegion(0, {});
    }
    DisplayOrientation displayOrientation = screenSession->GetScreenProperty().GetDisplayOrientation();
    if (displayMode == FoldDisplayMode::FULL || displayMode == FoldDisplayMode::COORDINATION) {
        switch (displayOrientation) {
            case DisplayOrientation::PORTRAIT:
            case DisplayOrientation::PORTRAIT_INVERTED: {
                liveCreaseRegion_ = GetStatusFullFoldCreaseRegion(true);
                break;
            }
            case DisplayOrientation::LANDSCAPE:
            case DisplayOrientation::LANDSCAPE_INVERTED: {
                liveCreaseRegion_ = GetStatusFullFoldCreaseRegion(false);
                break;
            }
            default: {
                TLOGE(WmsLogTag::DMS, "displayOrientation is invalid");
            }
        }
    }
    if (displayMode == FoldDisplayMode::GLOBAL_FULL) {
        switch (displayOrientation) {
            case DisplayOrientation::PORTRAIT:
            case DisplayOrientation::PORTRAIT_INVERTED: {
                liveCreaseRegion_ = GetStatusGlobalFullFoldCreaseRegion(false);
                break;
            }
            case DisplayOrientation::LANDSCAPE:
            case DisplayOrientation::LANDSCAPE_INVERTED: {
                liveCreaseRegion_ = GetStatusGlobalFullFoldCreaseRegion(true);
                break;
            }
            default: {
                TLOGE(WmsLogTag::DMS, "displayOrientation is invalid");
            }
        }
    }
    return liveCreaseRegion_;
}

void SecondaryDisplayFoldPolicy::InitScreenParams()
{
    std::vector<DisplayPhysicalResolution> resolutions = ScreenSceneConfig::GetAllDisplayPhysicalConfig();
    for (auto &resolution : resolutions) {
        if (FoldDisplayMode::MAIN == resolution.foldDisplayMode_) {
            screenParams_.push_back(resolution.physicalWidth_);
            screenParams_.push_back(resolution.physicalHeight_);
        } else if (FoldDisplayMode::FULL == resolution.foldDisplayMode_) {
            screenParams_.push_back(resolution.physicalWidth_);
            screenParams_.push_back(resolution.physicalHeight_);
        } else if (FoldDisplayMode::GLOBAL_FULL == resolution.foldDisplayMode_) {
            screenParams_.push_back(resolution.physicalWidth_);
            screenParams_.push_back(resolution.physicalHeight_);
        } else {
            TLOGW(WmsLogTag::DMS, "unKnown displayMode");
        }
    }
    // M mode offsetX
    screenParams_.push_back(screenParams_[GLOBAL_FULL_STATUS_WIDTH] - screenParams_[FULL_STATUS_WIDTH]);
    // M mode offsetY
    screenParams_.push_back(screenParams_[MAIN_STATUS_HEIGHT] - screenParams_[FULL_STATUS_HEIGHT]);
    // G mode offsetY
    screenParams_.push_back(screenParams_[MAIN_STATUS_HEIGHT] - screenParams_[SCREEN_HEIGHT]);
    TLOGI(WmsLogTag::DMS,
        "PhysicalResolution : mainStatusWidth_= %{public}d, fullStatusWidth_= %{public}d, gloablFullStatusWidth_="
        "%{public}d, screenHeight_= %{public}d, m_offsetX= %{public}d, m_offsetY= %{public}d, g_offsetY= %{public}d",
        screenParams_[MAIN_STATUS_WIDTH], screenParams_[FULL_STATUS_WIDTH],
        screenParams_[GLOBAL_FULL_STATUS_WIDTH], screenParams_[SCREEN_HEIGHT],
        screenParams_[FULL_STATUS_OFFSET_X], screenParams_[FULL_STATUS_OFFSET_Y],
        screenParams_[GLOBAL_FULL_STATUS_OFFSET_Y]);
}

std::vector<uint32_t> SecondaryDisplayFoldPolicy::GetScreenParams()
{
    return screenParams_;
}

void SecondaryDisplayFoldPolicy::SetSecondaryDisplayModeChangeStatus(bool status)
{
    if (status) {
        secondaryPengdingTask_ = SECONDARY_FOLD_TO_EXPAND_TASK_NUM;
        startTimePoint_ = std::chrono::steady_clock::now();
        displayModeChangeRunning_ = status;
    } else {
        secondaryPengdingTask_ --;
        if (secondaryPengdingTask_ > 0) {
            return;
        }
        displayModeChangeRunning_ = false;
        endTimePoint_ = std::chrono::steady_clock::now();
        if (lastCachedisplayMode_.load() != GetScreenDisplayMode()) {
            ScreenSessionManager::GetInstance().TriggerDisplayModeUpdate(lastCachedisplayMode_.load());
        }
    }
}

} // namespace OHOS::Rosen
