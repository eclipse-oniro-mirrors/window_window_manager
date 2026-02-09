/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "abstract_display_controller.h"

#include <cinttypes>
#include <hitrace_meter.h>
#include <sstream>
#include <surface.h>

#include "display_cutout_controller.h"
#include "display_manager_agent_controller.h"
#include "display_manager_service.h"
#include "screen_group.h"
#include "screen_rotation_controller.h"
#include "surface_capture_future.h"
#include "window_manager_hilog.h"
#include "sys_cap_util.h"

namespace OHOS::Rosen {

AbstractDisplayController::AbstractDisplayController(std::recursive_mutex& mutex, DisplayStateChangeListener listener)
    : mutex_(mutex), rsInterface_(RSInterfaces::GetInstance()), displayStateChangeListener_(listener)
{
}

AbstractDisplayController::~AbstractDisplayController()
{
    abstractScreenController_ = nullptr;
}

void AbstractDisplayController::Init(sptr<AbstractScreenController> abstractScreenController)
{
    TLOGD(WmsLogTag::DMS, "display controller init");
    displayCount_ = 0;
    abstractScreenController_ = abstractScreenController;
    abstractScreenCallback_ = new(std::nothrow) AbstractScreenController::AbstractScreenCallback();
    if (abstractScreenCallback_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "abstractScreenCallback init failed");
        return;
    }
    abstractScreenCallback_->onConnect_
        = std::bind(&AbstractDisplayController::OnAbstractScreenConnect, this, std::placeholders::_1);
    abstractScreenCallback_->onDisconnect_
        = std::bind(&AbstractDisplayController::OnAbstractScreenDisconnect, this, std::placeholders::_1);
    abstractScreenCallback_->onChange_
        = std::bind(&AbstractDisplayController::OnAbstractScreenChange, this, std::placeholders::_1,
        std::placeholders::_2);
    abstractScreenController->RegisterAbstractScreenCallback(abstractScreenCallback_);
}

ScreenId AbstractDisplayController::GetDefaultScreenId()
{
    return rsInterface_.GetDefaultScreenId();
}

RSScreenModeInfo AbstractDisplayController::GetScreenActiveMode(ScreenId id)
{
    return rsInterface_.GetScreenActiveMode(id);
}

sptr<AbstractDisplay> AbstractDisplayController::GetAbstractDisplay(DisplayId displayId) const
{
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "display id is invalid.");
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = abstractDisplayMap_.find(displayId);
    if (iter == abstractDisplayMap_.end()) {
        TLOGE(WmsLogTag::DMS, "Failed to get AbstractDisplay %{public}" PRIu64", return nullptr!", displayId);
        return nullptr;
    }
    return iter->second;
}

sptr<AbstractDisplay> AbstractDisplayController::GetAbstractDisplayByScreen(ScreenId screenId) const
{
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screen id is invalid.");
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto iter : abstractDisplayMap_) {
        sptr<AbstractDisplay> display = iter.second;
        if (display->GetAbstractScreenId() == screenId) {
            return display;
        }
    }
    TLOGE(WmsLogTag::DMS, "fail to get AbstractDisplay %{public}" PRIu64"", screenId);
    return nullptr;
}

std::vector<DisplayId> AbstractDisplayController::GetAllDisplayIds() const
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::vector<DisplayId> res;
    for (auto iter = abstractDisplayMap_.begin(); iter != abstractDisplayMap_.end(); ++iter) {
        res.push_back(iter->first);
    }
    return res;
}

std::shared_ptr<Media::PixelMap> AbstractDisplayController::GetScreenSnapshot(DisplayId displayId)
{
    sptr<AbstractDisplay> abstractDisplay = GetAbstractDisplay(displayId);
    if (abstractDisplay == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetScreenSnapshot: GetAbstractDisplay failed");
        return nullptr;
    }
    ScreenId dmsScreenId = abstractDisplay->GetAbstractScreenId();
    std::shared_ptr<RSDisplayNode> displayNode = abstractScreenController_->GetRSDisplayNodeByScreenId(dmsScreenId);

    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<SurfaceCaptureFuture> callback = std::make_shared<SurfaceCaptureFuture>();
    rsInterface_.TakeSurfaceCapture(displayNode, callback);
    std::shared_ptr<Media::PixelMap> screenshot = callback->GetResult(2000); // wait for <= 2000ms
    if (screenshot == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get pixelmap from RS, return nullptr!");
    }

    // notify dm listener
    sptr<ScreenshotInfo> snapshotInfo = new ScreenshotInfo();
    snapshotInfo->SetTrigger(SysCapUtil::GetClientName());
    snapshotInfo->SetDisplayId(displayId);
    DisplayManagerAgentController::GetInstance().OnScreenshot(snapshotInfo);

    return screenshot;
}

void AbstractDisplayController::OnAbstractScreenConnect(sptr<AbstractScreen> absScreen)
{
    if (absScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "absScreen is null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "connect new screen. id:%{public}" PRIu64"", absScreen->dmsId_);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    sptr<AbstractScreenGroup> group = absScreen->GetGroup();
    if (group == nullptr) {
        TLOGE(WmsLogTag::DMS, "the group information of the screen is wrong");
        return;
    }
    if (group->combination_ == ScreenCombination::SCREEN_ALONE || group->GetChildCount() == 1) {
        BindAloneScreenLocked(absScreen);
    } else if (group->combination_ == ScreenCombination::SCREEN_MIRROR) {
        TLOGI(WmsLogTag::DMS, "ScreenCombination::SCREEN_MIRROR, AddScreenToMirrorLocked");
        AddScreenToMirrorLocked(absScreen);
    } else if (group->combination_ == ScreenCombination::SCREEN_EXPAND) {
        TLOGI(WmsLogTag::DMS, "ScreenCombination::SCREEN_EXPAND, AddScreenToExpandLocked");
        AddScreenToExpandLocked(absScreen);
    } else {
        TLOGE(WmsLogTag::DMS, "support in future. combination:%{public}u", group->combination_);
    }
}

void AbstractDisplayController::OnAbstractScreenDisconnect(sptr<AbstractScreen> absScreen)
{
    if (absScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "the information of the screen is wrong");
        return;
    }
    TLOGI(WmsLogTag::DMS, "disconnect screen. id:%{public}" PRIu64"", absScreen->dmsId_);
    sptr<AbstractScreenGroup> screenGroup;
    DisplayId absDisplayId = DISPLAY_ID_INVALID;
    sptr<AbstractDisplay> abstractDisplay = nullptr;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    screenGroup = absScreen->GetGroup();
    if (screenGroup == nullptr) {
        TLOGE(WmsLogTag::DMS, "the group information of the screen is wrong");
        return;
    }
    if (screenGroup->combination_ == ScreenCombination::SCREEN_ALONE
        || screenGroup->combination_ == ScreenCombination::SCREEN_MIRROR) {
        absDisplayId = ProcessNormalScreenDisconnected(absScreen, screenGroup, abstractDisplay);
    } else if (screenGroup->combination_ == ScreenCombination::SCREEN_EXPAND) {
        absDisplayId = ProcessExpandScreenDisconnected(absScreen, screenGroup, abstractDisplay);
    } else {
        TLOGE(WmsLogTag::DMS, "support in future. combination:%{public}u", screenGroup->combination_);
    }
    if (absDisplayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "the displayId of the disconnected expand screen was not found");
        return;
    }
    if (screenGroup->combination_ == ScreenCombination::SCREEN_ALONE
        || screenGroup->combination_ == ScreenCombination::SCREEN_MIRROR) {
        if (screenGroup->GetChildCount() == 0) {
            abstractDisplayMap_.erase(absDisplayId);
            DisplayManagerAgentController::GetInstance().OnDisplayDestroy(absDisplayId);
        }
    } else if (screenGroup->combination_ == ScreenCombination::SCREEN_EXPAND) {
        SetDisplayStateChangeListener(abstractDisplay, DisplayStateChangeType::DESTROY);
        DisplayManagerAgentController::GetInstance().OnDisplayDestroy(absDisplayId);
        abstractDisplayMap_.erase(absDisplayId);
    } else {
        TLOGE(WmsLogTag::DMS, "support in future. combination:%{public}u", screenGroup->combination_);
    }
}

DisplayId AbstractDisplayController::ProcessNormalScreenDisconnected(
    sptr<AbstractScreen> absScreen, sptr<AbstractScreenGroup> screenGroup, sptr<AbstractDisplay>& absDisplay)
{
    TLOGI(WmsLogTag::DMS, "normal screen disconnect");
    if (absScreen == nullptr || screenGroup == nullptr) {
        TLOGE(WmsLogTag::DMS, "Invalid params as nullptr.");
        return DISPLAY_ID_INVALID;
    }
    ScreenId defaultScreenId = abstractScreenController_->GetDefaultAbstractScreenId();
    sptr<AbstractScreen> defaultScreen = abstractScreenController_->GetAbstractScreen(defaultScreenId);
    for (auto iter = abstractDisplayMap_.begin(); iter != abstractDisplayMap_.end(); iter++) {
        DisplayId displayId = iter->first;
        sptr<AbstractDisplay> abstractDisplay = iter->second;
        if (abstractDisplay->GetAbstractScreenId() == absScreen->dmsId_) {
            TLOGI(WmsLogTag::DMS, "normal screen disconnect, displayId: %{public}" PRIu64", "
                "screenId: %{public}" PRIu64"", displayId, abstractDisplay->GetAbstractScreenId());
            abstractDisplay->BindAbstractScreen(defaultScreen);
            absDisplay = abstractDisplay;
            return displayId;
        }
    }
    return DISPLAY_ID_INVALID;
}

DisplayId AbstractDisplayController::ProcessExpandScreenDisconnected(
    sptr<AbstractScreen> absScreen, sptr<AbstractScreenGroup> screenGroup, sptr<AbstractDisplay>& absDisplay)
{
    TLOGI(WmsLogTag::DMS, "expand screen disconnect");
    if (absScreen == nullptr || screenGroup == nullptr) {
        TLOGE(WmsLogTag::DMS, "Invalid params as nullptr.");
        return DISPLAY_ID_INVALID;
    }
    DisplayId displayId = DISPLAY_ID_INVALID;
    for (auto iter = abstractDisplayMap_.begin(); iter != abstractDisplayMap_.end(); iter++) {
        sptr<AbstractDisplay> abstractDisplay = iter->second;
        if (abstractDisplay->GetAbstractScreenId() == absScreen->dmsId_) {
            TLOGI(WmsLogTag::DMS, "expand screen disconnect, displayId: %{public}" PRIu64", "
                "screenId: %{public}" PRIu64"", displayId, abstractDisplay->GetAbstractScreenId());
            absDisplay = abstractDisplay;
            displayId = iter->first;
        } else {
            abstractDisplay->SetOffset(0, 0);
            auto screenId = abstractDisplay->GetAbstractScreenId();
            SetScreenOffsetInner(screenId);
        }
    }
    return displayId;
}

void AbstractDisplayController::OnAbstractScreenChange(sptr<AbstractScreen> absScreen, DisplayChangeEvent event)
{
    if (absScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "the information of the screen is wrong");
        return;
    }
    TLOGI(WmsLogTag::DMS, "screen changes. id:%{public}" PRIu64"", absScreen->dmsId_);
    if (event == DisplayChangeEvent::UPDATE_ORIENTATION) {
        ProcessDisplayUpdateOrientation(absScreen, DisplayStateChangeType::UPDATE_ROTATION);
    } else if (event == DisplayChangeEvent::UPDATE_ORIENTATION_FROM_WINDOW) {
        ProcessDisplayUpdateOrientation(absScreen, DisplayStateChangeType::UPDATE_ROTATION_FROM_WINDOW);
    } else if (event == DisplayChangeEvent::DISPLAY_SIZE_CHANGED) {
        ProcessDisplaySizeChange(absScreen);
    } else if (event == DisplayChangeEvent::DISPLAY_VIRTUAL_PIXEL_RATIO_CHANGED) {
        ProcessVirtualPixelRatioChange(absScreen);
    } else if (event == DisplayChangeEvent::UPDATE_ROTATION) {
        ProcessDisplayRotationChange(absScreen, DisplayStateChangeType::UPDATE_ROTATION);
    } else if (event == DisplayChangeEvent::UPDATE_ROTATION_FROM_WINDOW) {
        ProcessDisplayRotationChange(absScreen, DisplayStateChangeType::UPDATE_ROTATION_FROM_WINDOW);
    } else {
        TLOGE(WmsLogTag::DMS, "unknown screen change event. id:%{public}" PRIu64" event %{public}u",
            absScreen->dmsId_, event);
    }
}

void AbstractDisplayController::ProcessDisplayRotationChange(sptr<AbstractScreen> absScreen,
    DisplayStateChangeType type)
{
    if (absScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "absScreen is nullptr");
        return;
    }
    sptr<AbstractDisplay> abstractDisplay = GetAbstractDisplayByAbsScreen(absScreen);
    if (abstractDisplay == nullptr) {
        return;
    }
    if (abstractDisplay->RequestRotation(absScreen->rotation_)) {
        abstractDisplay->SetDisplayOrientation(
            ScreenRotationController::ConvertRotationToDisplayOrientation(absScreen->rotation_));
        // Notify rotation event to WMS
        SetDisplayStateChangeListener(abstractDisplay, type);
    }
    sptr<DisplayInfo> displayInfo = abstractDisplay->ConvertToDisplayInfo();
    DisplayManagerAgentController::GetInstance().OnDisplayChange(displayInfo,
        DisplayChangeEvent::UPDATE_ROTATION);
    ProcessDisplayCompression(absScreen);
}

void AbstractDisplayController::ProcessDisplayCompression(sptr<AbstractScreen> absScreen)
{
    TLOGI(WmsLogTag::DMS, "Enter");
    auto absDisplay = GetAbstractDisplayByAbsScreen(absScreen);
    DisplayId defaultDisplayId = GetDefaultDisplayId();
    if (absDisplay == nullptr || absDisplay->GetId() != defaultDisplayId) {
        return;
    }
    uint32_t sizeInVp = DisplayCutoutController::GetWaterfallAreaCompressionSizeWhenHorizontal();
    if (!DisplayCutoutController::IsWaterfallAreaCompressionEnableWhenHorizontal() || sizeInVp == 0) {
        TLOGI(WmsLogTag::DMS, "Not enable waterfall display area compression.");
        return;
    }
    auto mode = absScreen->GetActiveScreenMode();
    if (mode == nullptr) {
        TLOGW(WmsLogTag::DMS, "SupportedScreenModes is null");
        return;
    }
    uint32_t screenHeight = mode->height_;
    uint32_t screenWidth = mode->width_;
    uint32_t sizeInPx = static_cast<uint32_t>(sizeInVp * absDisplay->GetVirtualPixelRatio());
    // 4: Compression size shall less than 1/4 of the screen size.
    if (sizeInPx >= screenHeight / 4 || sizeInPx >= screenWidth / 4) {
        TLOGW(WmsLogTag::DMS, "Invalid value for waterfall display curved area avoid size of each sides");
        return;
    }
    TLOGI(WmsLogTag::DMS, "SizeInPx: %{public}u", sizeInPx);
    Rotation rotation = absDisplay->GetRotation();
    bool isDefaultRotationVertical = mode->height_ > mode->width_ ? true : false;
    if (ScreenRotationController::IsDisplayRotationHorizontal(rotation)) {
        uint32_t offsetY = sizeInPx;
        uint32_t totalCompressedSize = offsetY * 2; // *2 for both sides.
        uint32_t displayHeightAfter = isDefaultRotationVertical ? mode->width_ - totalCompressedSize :
            mode->height_ - totalCompressedSize;
        absDisplay->SetOffsetX(0);
        absDisplay->SetOffsetY(offsetY);
        absDisplay->SetHeight(displayHeightAfter);
        absDisplay->SetWaterfallDisplayCompressionStatus(true);
    } else {
        if (!absDisplay->GetWaterfallDisplayCompressionStatus()) {
            return;
        }
        absDisplay->SetOffsetX(0);
        absDisplay->SetOffsetY(0);
        absDisplay->SetHeight(isDefaultRotationVertical ? mode->height_ : mode->width_);
        absDisplay->SetWidth(isDefaultRotationVertical ? mode->width_ : mode->height_);
        absDisplay->SetWaterfallDisplayCompressionStatus(false);
    }
    SetDisplayStateChangeListener(absDisplay, DisplayStateChangeType::DISPLAY_COMPRESS);
    DisplayManagerAgentController::GetInstance().OnDisplayChange(
        absDisplay->ConvertToDisplayInfo(), DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
}

sptr<AbstractDisplay> AbstractDisplayController::GetAbstractDisplayByAbsScreen(sptr<AbstractScreen> absScreen)
{
    sptr<AbstractDisplay> abstractDisplay = nullptr;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = abstractDisplayMap_.begin();
    for (; iter != abstractDisplayMap_.end(); iter++) {
        if (iter->second->GetAbstractScreenId() == absScreen->dmsId_) {
            abstractDisplay = iter->second;
            TLOGD(WmsLogTag::DMS, "find abstract display of the screen. display %{public}" PRIu64", "
                "screen %{public}" PRIu64"", abstractDisplay->GetId(), absScreen->dmsId_);
            break;
        }
    }
    sptr<AbstractScreenGroup> group = absScreen->GetGroup();
    if (group == nullptr) {
        TLOGE(WmsLogTag::DMS, "cannot get screen group");
        return nullptr;
    }
    if (iter == abstractDisplayMap_.end()) {
        if (group->combination_ == ScreenCombination::SCREEN_ALONE
            || group->combination_ == ScreenCombination::SCREEN_EXPAND) {
            TLOGE(WmsLogTag::DMS, "Screen combination is SCREEN_ALONE or SCREEN_EXPAND, "
                "cannot find abstract display of the screen");
        } else if (group->combination_ == ScreenCombination::SCREEN_MIRROR) {
            // If the screen cannot be found in 'abstractDisplayMap_', it means that the screen is the secondary
            TLOGI(WmsLogTag::DMS, "It's the secondary screen of the mirrored.");
        } else {
            TLOGE(WmsLogTag::DMS, "Unknown combination");
        }
        return nullptr;
    }
    return abstractDisplay;
}

void AbstractDisplayController::ProcessDisplayUpdateOrientation(sptr<AbstractScreen> absScreen,
    DisplayStateChangeType type)
{
    sptr<AbstractDisplay> abstractDisplay = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto iter = abstractDisplayMap_.begin();
        for (; iter != abstractDisplayMap_.end(); iter++) {
            abstractDisplay = iter->second;
            if (abstractDisplay->GetAbstractScreenId() == absScreen->dmsId_) {
                TLOGD(WmsLogTag::DMS, "find abstract display of the screen. display %{public}" PRIu64", "
                    "screen %{public}" PRIu64"", abstractDisplay->GetId(), absScreen->dmsId_);
                break;
            }
        }

        sptr<AbstractScreenGroup> group = absScreen->GetGroup();
        if (group == nullptr) {
            TLOGE(WmsLogTag::DMS, "cannot get screen group");
            return;
        }
        if (iter == abstractDisplayMap_.end()) {
            if (group->combination_ == ScreenCombination::SCREEN_ALONE
                || group->combination_ == ScreenCombination::SCREEN_EXPAND) {
                TLOGE(WmsLogTag::DMS, "cannot find abstract display of the screen %{public}" PRIu64"",
                    absScreen->dmsId_);
                return;
            } else if (group->combination_ == ScreenCombination::SCREEN_MIRROR) {
                // If the screen cannot be found in 'abstractDisplayMap_', it means that the screen is the secondary
                TLOGI(WmsLogTag::DMS, "It's the secondary screen of the mirrored.");
                return;
            } else {
                TLOGE(WmsLogTag::DMS, "Unknown combination");
                return;
            }
        }
    }
    abstractDisplay->SetOrientation(absScreen->orientation_);
    if (abstractDisplay->RequestRotation(absScreen->rotation_)) {
        // Notify rotation event to WMS
        SetDisplayStateChangeListener(abstractDisplay, type);
    }
}

void AbstractDisplayController::ProcessDisplaySizeChange(sptr<AbstractScreen> absScreen)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:ProcessDisplaySizeChange(%" PRIu64")", absScreen->dmsId_);
    sptr<SupportedScreenModes> info = absScreen->GetActiveScreenMode();
    if (info == nullptr) {
        TLOGE(WmsLogTag::DMS, "cannot get active screen info.");
        return;
    }

    std::map<DisplayId, sptr<AbstractDisplay>> matchedDisplays;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        for (auto iter = abstractDisplayMap_.begin(); iter != abstractDisplayMap_.end(); ++iter) {
            sptr<AbstractDisplay> absDisplay = iter->second;
            if (absDisplay == nullptr || absDisplay->GetAbstractScreenId() != absScreen->dmsId_) {
                continue;
            }
            if (UpdateDisplaySize(absDisplay, info, absScreen->startPoint_)) {
                matchedDisplays.insert(std::make_pair(iter->first, iter->second));
            }
        }
    }

    TLOGI(WmsLogTag::DMS, "Size of matchedDisplays %{public}zu", matchedDisplays.size());
    for (auto iter = matchedDisplays.begin(); iter != matchedDisplays.end(); ++iter) {
        TLOGI(WmsLogTag::DMS, "Notify display size change. Id %{public}" PRIu64"", iter->first);
        sptr<AbstractDisplay> abstractDisplay = iter->second;
        SetDisplayStateChangeListener(abstractDisplay, DisplayStateChangeType::SIZE_CHANGE);
        DisplayManagerAgentController::GetInstance().OnDisplayChange(
            abstractDisplay->ConvertToDisplayInfo(), DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
    }
}

bool AbstractDisplayController::UpdateDisplaySize(sptr<AbstractDisplay> absDisplay, sptr<SupportedScreenModes> info,
    Point offset)
{
    if (absDisplay == nullptr) {
        TLOGE(WmsLogTag::DMS, "invalid params.");
        return false;
    }

    bool changed = false;
    if (info) {
        auto rotation = absDisplay->GetRotation();
        int32_t width = 0;
        int32_t height = 0;
        if (rotation == Rotation::ROTATION_90 || rotation == Rotation::ROTATION_270) {
            width = absDisplay->GetHeight();
            height = absDisplay->GetWidth();
        } else {
            width = absDisplay->GetWidth();
            height = absDisplay->GetHeight();
        }

        if (info->width_ == static_cast<uint32_t>(width) &&
            info->height_ == static_cast<uint32_t>(height)) {
            TLOGD(WmsLogTag::DMS, "keep display size. display:%{public}" PRIu64"", absDisplay->GetId());
        } else {
            TLOGD(WmsLogTag::DMS, "Reset H&W. id %{public}" PRIu64", size: %{public}d %{public}d",
                absDisplay->GetId(), info->width_, info->height_);
            absDisplay->SetWidth(info->width_);
            absDisplay->SetHeight(info->height_);
            changed = true;
        }
    } else {
        TLOGE(WmsLogTag::DMS, "mode info is null");
    }

    if (offset.posX_ == absDisplay->GetOffsetX() &&
        offset.posY_ == absDisplay->GetOffsetY()) {
        TLOGD(WmsLogTag::DMS, "keep display offset. display:%{public}" PRIu64"", absDisplay->GetId());
    } else {
        TLOGD(WmsLogTag::DMS, "Reset offset. id %{public}" PRIu64", size: %{public}d %{public}d",
            absDisplay->GetId(), offset.posX_, offset.posY_);
        absDisplay->SetOffsetX(offset.posX_);
        absDisplay->SetOffsetY(offset.posY_);
        changed = true;
    }
    
    return changed;
}

void AbstractDisplayController::ProcessVirtualPixelRatioChange(sptr<AbstractScreen> absScreen)
{
    sptr<AbstractDisplay> abstractDisplay = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto iter = abstractDisplayMap_.begin();
        for (; iter != abstractDisplayMap_.end(); iter++) {
            abstractDisplay = iter->second;
            if (abstractDisplay->GetAbstractScreenId() == absScreen->dmsId_) {
                TLOGD(WmsLogTag::DMS, "find abstract display of the screen. display %{public}" PRIu64", "
                    "screen %{public}" PRIu64"", abstractDisplay->GetId(), absScreen->dmsId_);
                break;
            }
        }
    }
    if (abstractDisplay == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to find abstract display of the screen.");
        return;
    }
    abstractDisplay->SetVirtualPixelRatio(absScreen->virtualPixelRatio_);
    // Notify virtual pixel ratio change event to WMS
    SetDisplayStateChangeListener(abstractDisplay, DisplayStateChangeType::VIRTUAL_PIXEL_RATIO_CHANGE);
    // Notify virtual pixel ratio change event to DisplayManager
    DisplayManagerAgentController::GetInstance().OnDisplayChange(abstractDisplay->ConvertToDisplayInfo(),
        DisplayChangeEvent::DISPLAY_VIRTUAL_PIXEL_RATIO_CHANGED);
}

void AbstractDisplayController::BindAloneScreenLocked(sptr<AbstractScreen> realAbsScreen)
{
    if (realAbsScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "BindAloneScreenLocked failed, realAbsScreen is nullptr");
        return;
    }
    ScreenId defaultScreenId = abstractScreenController_->GetDefaultAbstractScreenId();
    if (defaultScreenId != SCREEN_ID_INVALID) {
        if (defaultScreenId != realAbsScreen->dmsId_) {
            TLOGE(WmsLogTag::DMS, "The first real screen should be default for Phone. %{public}" PRIu64"",
                realAbsScreen->dmsId_);
            return;
        }
        sptr<SupportedScreenModes> info = realAbsScreen->GetActiveScreenMode();
        if (info == nullptr) {
            TLOGE(WmsLogTag::DMS, "bind alone screen error, cannot get info.");
            return;
        }
        if (dummyDisplay_ == nullptr) {
            DisplayId displayId = displayCount_.fetch_add(1);
            sptr<AbstractDisplay> display = new(std::nothrow) AbstractDisplay(displayId, info, realAbsScreen);
            if (display == nullptr) {
                TLOGE(WmsLogTag::DMS, "create display failed");
                return;
            }

            abstractDisplayMap_.insert((std::make_pair(display->GetId(), display)));
            TLOGI(WmsLogTag::DMS, "create display for new screen. screen:%{public}" PRIu64", "
                "display:%{public}" PRIu64"", realAbsScreen->dmsId_, display->GetId());
            DisplayManagerAgentController::GetInstance().OnDisplayCreate(display->ConvertToDisplayInfo());
            SetDisplayStateChangeListener(display, DisplayStateChangeType::CREATE);
        } else {
            TLOGI(WmsLogTag::DMS, "bind display for new screen. screen:%{public}" PRIu64", "
                "display:%{public}" PRIu64"", realAbsScreen->dmsId_, dummyDisplay_->GetId());
            bool updateFlag = static_cast<uint32_t>(dummyDisplay_->GetHeight()) == info->height_
                    && static_cast<uint32_t>(dummyDisplay_->GetWidth()) == info->width_;
            dummyDisplay_->BindAbstractScreen(abstractScreenController_->GetAbstractScreen(realAbsScreen->dmsId_));
            if (updateFlag) {
                DisplayManagerAgentController::GetInstance().OnDisplayCreate(dummyDisplay_->ConvertToDisplayInfo());
            }
            dummyDisplay_ = nullptr;
        }
    } else {
        TLOGE(WmsLogTag::DMS, "The first real screen should be default screen for Phone. %{public}" PRIu64"",
            realAbsScreen->dmsId_);
    }
}

void AbstractDisplayController::AddScreenToMirrorLocked(sptr<AbstractScreen> absScreen)
{
    TLOGI(WmsLogTag::DMS, "bind display to mirror. screen:%{public}" PRIu64"", absScreen->dmsId_);
}

void AbstractDisplayController::AddScreenToExpandLocked(sptr<AbstractScreen> absScreen)
{
    if (absScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "AddScreenToExpandLocked failed, absScreen is nullptr");
        return;
    }
    for (auto iter = abstractDisplayMap_.begin(); iter != abstractDisplayMap_.end(); iter++) {
        sptr<AbstractDisplay> abstractDisplay = iter->second;
        if (abstractDisplay->GetAbstractScreenId() == absScreen->dmsId_) {
            TLOGE(WmsLogTag::DMS, "error, screenId: %{public}" PRIu64" already has corresponding display",
                absScreen->dmsId_);
            return;
        }
    }
    TLOGI(WmsLogTag::DMS, "bind display to expand. screen:%{public}" PRIu64"", absScreen->dmsId_);
    sptr<SupportedScreenModes> info;
    ScreenId defaultScreenId = abstractScreenController_->GetDefaultAbstractScreenId();
    sptr<AbstractScreen> defaultScreen = abstractScreenController_->GetAbstractScreen(defaultScreenId);
    if (absScreen->type_ == ScreenType::VIRTUAL) {
        TLOGI(WmsLogTag::DMS, "screen type is virtual, use default screen info");
        if (defaultScreen == nullptr) {
            TLOGE(WmsLogTag::DMS, "bind display error, cannot get defaultScreen.");
            return;
        }
        info = defaultScreen->GetActiveScreenMode();
    } else {
        TLOGI(WmsLogTag::DMS, "screen type is not virtual, get this screen info");
        info = absScreen->GetActiveScreenMode();
    }
    if (info == nullptr) {
        TLOGE(WmsLogTag::DMS, "bind display error, cannot get info.");
        return;
    }
    DisplayId displayId = displayCount_.fetch_add(1);
    sptr<AbstractDisplay> display = new AbstractDisplay(displayId, info, absScreen);
    Point point = abstractScreenController_->GetAbstractScreenGroup(absScreen->groupDmsId_)->
        GetChildPosition(absScreen->dmsId_);
    display->SetOffset(point.posX_, point.posY_);
    abstractDisplayMap_.insert((std::make_pair(display->GetId(), display)));
    TLOGI(WmsLogTag::DMS, "create display for new screen. screen:%{public}" PRIu64", "
        "display:%{public}" PRIu64"", absScreen->dmsId_, display->GetId());
    DisplayManagerAgentController::GetInstance().OnDisplayCreate(display->ConvertToDisplayInfo());
    SetDisplayStateChangeListener(display, DisplayStateChangeType::CREATE);
}

void AbstractDisplayController::SetFreeze(std::vector<DisplayId> displayIds, bool toFreeze)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:SetAllFreeze");
    DisplayStateChangeType type = toFreeze ? DisplayStateChangeType::FREEZE : DisplayStateChangeType::UNFREEZE;
    DisplayChangeEvent event
        = toFreeze ? DisplayChangeEvent::DISPLAY_FREEZED : DisplayChangeEvent::DISPLAY_UNFREEZED;
    for (DisplayId displayId : displayIds) {
        sptr<AbstractDisplay> abstractDisplay;
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:SetFreeze(%" PRIu64")", displayId);
        {
            TLOGI(WmsLogTag::DMS, "setfreeze display %{public}" PRIu64"", displayId);
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            auto iter = abstractDisplayMap_.find(displayId);
            if (iter == abstractDisplayMap_.end()) {
                TLOGE(WmsLogTag::DMS, "setfreeze fail, cannot get display %{public}" PRIu64"", displayId);
                continue;
            }
            abstractDisplay = iter->second;
            FreezeFlag curFlag = abstractDisplay->GetFreezeFlag();
            if ((toFreeze && (curFlag == FreezeFlag::FREEZING))
                || (!toFreeze && (curFlag == FreezeFlag::UNFREEZING))) {
                TLOGE(WmsLogTag::DMS, "setfreeze fail, display %{public}" PRIu64" freezeflag is %{public}u",
                    displayId, curFlag);
                continue;
            }
            FreezeFlag flag = toFreeze ? FreezeFlag::FREEZING : FreezeFlag::UNFREEZING;
            abstractDisplay->SetFreezeFlag(flag);
        }

        // Notify freeze event to WMS
        SetDisplayStateChangeListener(abstractDisplay, type);
        // Notify freeze event to DisplayManager
        DisplayManagerAgentController::GetInstance().OnDisplayChange(abstractDisplay->ConvertToDisplayInfo(), event);
    }
}

std::map<DisplayId, sptr<DisplayInfo>> AbstractDisplayController::GetAllDisplayInfoOfGroup(sptr<DisplayInfo> info)
{
    ScreenId screenGroupId = info->GetScreenGroupId();
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (const auto& iter : abstractDisplayMap_) {
        sptr<AbstractDisplay> display = iter.second;
        if (display->GetAbstractScreenGroupId() == screenGroupId) {
            displayInfoMap.insert(std::make_pair(display->GetId(), display->ConvertToDisplayInfo()));
        }
    }
    return displayInfoMap;
}

void AbstractDisplayController::SetDisplayStateChangeListener(
    sptr<AbstractDisplay> abstractDisplay, DisplayStateChangeType type)
{
    ScreenId defaultDisplayId = GetDefaultDisplayId();
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap = GetAllDisplayInfoOfGroup(
        abstractDisplay->ConvertToDisplayInfo());
    displayStateChangeListener_(defaultDisplayId, abstractDisplay->ConvertToDisplayInfo(), displayInfoMap, type);
}

DisplayId AbstractDisplayController::GetDefaultDisplayId()
{
    DisplayId defaultDisplayId = DISPLAY_ID_INVALID;
    ScreenId defaultScreenId = abstractScreenController_->GetDefaultAbstractScreenId();
    sptr<AbstractDisplay> defaultDisplay = GetAbstractDisplayByScreen(defaultScreenId);
    if (defaultDisplay != nullptr) {
        defaultDisplayId = defaultDisplay->GetId();
    }
    return defaultDisplayId;
}

bool AbstractDisplayController::SetScreenOffsetInner(ScreenId screenId)
{
    sptr<AbstractScreen> absScreen = abstractScreenController_->GetAbstractScreen(screenId);
    if (absScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "did not find screen:%{public}" PRIu64, screenId);
        return false;
    }
    TLOGI(WmsLogTag::DMS, "screenId = %{public}" PRIu64 " rsId = %{public}" PRIu64, screenId, absScreen->rsId_);
    RSInterfaces::GetInstance().SetScreenOffset(absScreen->rsId_, 0, 0);
    return true;
}
} // namespace OHOS::Rosen