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

#include "abstract_screen_controller.h"

#include <sstream>

#include <cinttypes>
#include <hitrace_meter.h>
#include <parameters.h>
#include <screen_manager/rs_screen_mode_info.h>
#include <screen_manager/screen_types.h>
#include <surface.h>
#include <thread>

#include "sys_cap_util.h"
#include "display_manager_agent_controller.h"
#include "display_manager_service.h"
#include "event_runner.h"
#include "screen_rotation_controller.h"
#include "window_manager_hilog.h"

#ifdef SOC_PERF_ENABLE
#include "socperf_client.h"
#endif

namespace OHOS::Rosen {
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "AbstractScreenController"};
    const std::string CONTROLLER_THREAD_ID = "AbstractScreenControllerThread";
}

AbstractScreenController::AbstractScreenController(std::recursive_mutex& mutex)
    : mutex_(mutex), rsInterface_(RSInterfaces::GetInstance())
{
    auto runner = AppExecFwk::EventRunner::Create(CONTROLLER_THREAD_ID);
    controllerHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
}

AbstractScreenController::~AbstractScreenController() = default;

void AbstractScreenController::Init()
{
    WLOGFD("screen controller init");
    RegisterRsScreenConnectionChangeListener();
}

void AbstractScreenController::RegisterRsScreenConnectionChangeListener()
{
    WLOGFD("RegisterRsScreenConnectionChangeListener");
    auto res = rsInterface_.SetScreenChangeCallback(
        [this](ScreenId rsScreenId, ScreenEvent screenEvent) { OnRsScreenConnectionChange(rsScreenId, screenEvent); });
    if (res != StatusCode::SUCCESS) {
        auto task = [this] {
            RegisterRsScreenConnectionChangeListener();
        };
        // post task after 50 ms.
        controllerHandler_->PostTask(task, 50, AppExecFwk::EventQueue::Priority::HIGH);
    }
}

std::vector<ScreenId> AbstractScreenController::GetAllScreenIds() const
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::vector<ScreenId> res;
    for (const auto& iter : dmsScreenMap_) {
        res.emplace_back(iter.first);
    }
    return res;
}

std::vector<ScreenId> AbstractScreenController::GetAllValidScreenIds(const std::vector<ScreenId>& screenIds) const
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::vector<ScreenId> validScreenIds;
    for (ScreenId screenId : screenIds) {
        auto screenIdIter = std::find(validScreenIds.begin(), validScreenIds.end(), screenId);
        if (screenIdIter != validScreenIds.end()) {
            continue;
        }
        auto iter = dmsScreenMap_.find(screenId);
        if (iter != dmsScreenMap_.end() && iter->second->type_ != ScreenType::UNDEFINED) {
            validScreenIds.emplace_back(screenId);
        }
    }
    return validScreenIds;
}

const std::shared_ptr<RSDisplayNode>& AbstractScreenController::GetRSDisplayNodeByScreenId(ScreenId dmsScreenId) const
{
    static std::shared_ptr<RSDisplayNode> notFound = nullptr;
    sptr<AbstractScreen> screen = GetAbstractScreen(dmsScreenId);
    if (screen == nullptr) {
        return notFound;
    }
    if (screen->rsDisplayNode_ == nullptr) {
        return notFound;
    }
    WLOGI("GetRSDisplayNodeByScreenId: screen: %{public}" PRIu64", nodeId: %{public}" PRIu64" ",
        screen->dmsId_, screen->rsDisplayNode_->GetId());
    return screen->rsDisplayNode_;
}

void AbstractScreenController::UpdateRSTree(ScreenId dmsScreenId, ScreenId parentScreenId,
    std::shared_ptr<RSSurfaceNode>& surfaceNode, bool isAdd, bool isMultiDisplay)
{
    sptr<AbstractScreen> abstractScreen = GetAbstractScreen(dmsScreenId);
    if (abstractScreen == nullptr) {
        WLOGE("[UpdateRSTree] can not find abstractScreen");
        return;
    }
    if (isMultiDisplay) {
        sptr<AbstractScreen> parentAbstractScreen = GetAbstractScreen(parentScreenId);
        if (parentAbstractScreen == nullptr) {
            WLOGE("[UpdateRSTree] can not find parentAbstractScreen");
            return;
        }
        if (parentAbstractScreen->rsDisplayNode_ == nullptr) {
            WLOGE("rsDisplayNode of parentAbstractScreen is nullptr");
            return;
        }
        abstractScreen->UpdateDisplayGroupRSTree(surfaceNode, parentAbstractScreen->rsDisplayNode_->GetId(), isAdd);
    } else {
        abstractScreen->UpdateRSTree(surfaceNode, isAdd);
    }
}

DMError AbstractScreenController::AddSurfaceNodeToScreen(ScreenId dmsScreenId,
    std::shared_ptr<RSSurfaceNode>& surfaceNode, bool onTop)
{
    sptr<AbstractScreen> abstractScreen = GetAbstractScreen(dmsScreenId);
    if (abstractScreen == nullptr) {
        WLOGFE("Can not find abstractScreen");
        return DMError::DM_ERROR_NULLPTR;
    }
    return abstractScreen->AddSurfaceNode(surfaceNode, onTop);
}

DMError AbstractScreenController::RemoveSurfaceNodeFromScreen(ScreenId dmsScreenId,
    std::shared_ptr<RSSurfaceNode>& surfaceNode)
{
    sptr<AbstractScreen> abstractScreen = GetAbstractScreen(dmsScreenId);
    if (abstractScreen == nullptr) {
        WLOGFE("Can not find abstractScreen");
        return DMError::DM_ERROR_NULLPTR;
    }
    return abstractScreen->RemoveSurfaceNode(surfaceNode);
}

sptr<AbstractScreen> AbstractScreenController::GetAbstractScreen(ScreenId dmsScreenId) const
{
    WLOGD("GetAbstractScreen: screenId: %{public}" PRIu64"", dmsScreenId);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = dmsScreenMap_.find(dmsScreenId);
    if (iter == dmsScreenMap_.end()) {
        WLOGE("did not find screen:%{public}" PRIu64"", dmsScreenId);
        return nullptr;
    }
    return iter->second;
}

sptr<AbstractScreenGroup> AbstractScreenController::GetAbstractScreenGroup(ScreenId dmsScreenId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = dmsScreenGroupMap_.find(dmsScreenId);
    if (iter == dmsScreenGroupMap_.end()) {
        WLOGE("did not find screen:%{public}" PRIu64"", dmsScreenId);
        return nullptr;
    }
    return iter->second;
}

ScreenId AbstractScreenController::GetDefaultAbstractScreenId()
{
    if (defaultRsScreenId_ == SCREEN_ID_INVALID) {
        defaultRsScreenId_ = rsInterface_.GetDefaultScreenId();
    }
    if (defaultRsScreenId_ == SCREEN_ID_INVALID) {
        WLOGFW("GetDefaultAbstractScreenId, rsDefaultId is invalid.");
        return SCREEN_ID_INVALID;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    ScreenId defaultDmsScreenId;
    if (screenIdManager_.ConvertToDmsScreenId(defaultRsScreenId_, defaultDmsScreenId)) {
        WLOGFD("GetDefaultAbstractScreenId, screen:%{public}" PRIu64"", defaultDmsScreenId);
        return defaultDmsScreenId;
    }
    WLOGFI("GetDefaultAbstractScreenId, default screen is null, try to get.");
    ProcessScreenConnected(defaultRsScreenId_);
    return screenIdManager_.ConvertToDmsScreenId(defaultRsScreenId_);
}

ScreenId AbstractScreenController::ConvertToRsScreenId(ScreenId dmsScreenId) const
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return screenIdManager_.ConvertToRsScreenId(dmsScreenId);
}

ScreenId AbstractScreenController::ConvertToDmsScreenId(ScreenId rsScreenId) const
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return screenIdManager_.ConvertToDmsScreenId(rsScreenId);
}

void AbstractScreenController::RegisterAbstractScreenCallback(sptr<AbstractScreenCallback> cb)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    abstractScreenCallback_ = cb;
    if (abstractScreenCallback_ == nullptr) {
        return;
    }
    for (auto& iter : dmsScreenMap_) {
        if (iter.second != nullptr) {
            WLOGFI("dmsScreenId :%{public}" PRIu64"", iter.first);
            abstractScreenCallback_->onConnect_(iter.second);
        }
    }
}

void AbstractScreenController::OnRsScreenConnectionChange(ScreenId rsScreenId, ScreenEvent screenEvent)
{
    WLOGFI("RS screen event. rsScreenId:%{public}" PRIu64", defaultRsScreenId_:%{public}" PRIu64", event:%{public}u",
        rsScreenId, static_cast<uint64_t>(defaultRsScreenId_), static_cast<uint32_t>(screenEvent));
    if (screenEvent == ScreenEvent::CONNECTED) {
        auto task = [this, rsScreenId] {
            ProcessScreenConnected(rsScreenId);
        };
        controllerHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
    } else if (screenEvent == ScreenEvent::DISCONNECTED) {
        auto task = [this, rsScreenId] {
            ProcessScreenDisconnected(rsScreenId);
        };
        controllerHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
    } else {
        WLOGE("unknown message:%{public}ud", static_cast<uint8_t>(screenEvent));
    }
}

void AbstractScreenController::ProcessDefaultScreenReconnected(ScreenId rsScreenId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (rsScreenId != defaultRsScreenId_ || defaultRsScreenId_ == SCREEN_ID_INVALID) {
        return;
    }
    ScreenId dmsScreenId;
    if (!screenIdManager_.ConvertToDmsScreenId(rsScreenId, dmsScreenId)) {
        WLOGFE("disconnect screen, rsScreenId=%{public}" PRIu64" is not in rs2DmsScreenIdMap_", rsScreenId);
        return;
    }
    WLOGFD("rsScreenId=%{public}" PRIu64", dmsScreenId=%{public}" PRIu64", "
        "defaultRsScreenId: %{public}" PRIu64"", rsScreenId, dmsScreenId, static_cast<uint64_t>(defaultRsScreenId_));
    auto dmsScreenMapIter = dmsScreenMap_.find(dmsScreenId);
    if (dmsScreenMapIter != dmsScreenMap_.end()) {
        auto screen = dmsScreenMapIter->second;
        if (screen == nullptr) {
            WLOGFE("screen is nullptr");
            return;
        }
        auto groupDmsId = screen->lastGroupDmsId_;
        auto iter = dmsScreenGroupMap_.find(groupDmsId);
        if (iter == dmsScreenGroupMap_.end()) {
            WLOGFE("groupDmsId: %{public}" PRIu64"is not in dmsScreenGroupMap_.", groupDmsId);
            return;
        }
        sptr<AbstractScreenGroup> screenGroup = iter->second;
        if (screenGroup == nullptr) {
            WLOGFE("screenGroup is nullptr");
            return;
        }
        Point point;
        if (!screenGroup->AddChild(screen, point)) {
            WLOGE("fail to add screen to group. screen: %{public}" PRIu64"", screen->dmsId_);
            return;
        }

        // Recover default screen, set power state again
        SetScreenPowerForAll(powerState_, PowerStateChangeReason::POWER_BUTTON, false);
        const uint32_t level = 165;
        RSInterfaces::GetInstance().SetScreenBacklight(rsScreenId, level);
    } else {
        WLOGFE("can't find screen in dmsScreenMap, dmsScreenId: %{public}" PRIu64"", dmsScreenId);
    }
}

void AbstractScreenController::ProcessScreenConnected(ScreenId rsScreenId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (screenIdManager_.HasRsScreenId(rsScreenId)) {
        WLOGFD("reconnect screen, screenId=%{public}" PRIu64"", rsScreenId);
        ProcessDefaultScreenReconnected(rsScreenId);
        return;
    }
    auto absScreen = InitAndGetScreen(rsScreenId);
    if (absScreen == nullptr) {
        return;
    }
    sptr<AbstractScreenGroup> screenGroup = AddToGroupLocked(absScreen);
    if (screenGroup == nullptr) {
        return;
    }
    if (rsScreenId == rsInterface_.GetDefaultScreenId() && absScreen->rsDisplayNode_ != nullptr) {
        absScreen->screenRequestedOrientation_ = buildInDefaultOrientation_;
        Rotation rotationAfter = absScreen->CalcRotation(absScreen->screenRequestedOrientation_);
        WLOGFD("set default rotation to %{public}d for buildin screen", rotationAfter);
        sptr<SupportedScreenModes> abstractScreenModes = absScreen->GetActiveScreenMode();
        if (abstractScreenModes != nullptr) {
            float w = abstractScreenModes->width_;
            float h = abstractScreenModes->height_;
            float x = 0;
            float y = 0;
            if (!IsVertical(rotationAfter)) {
                std::swap(w, h);
                x = (h - w) / 2; // 2: used to calculate offset to center display node
                y = (w - h) / 2; // 2: used to calculate offset to center display node
            }
            // 90.f is base degree
            absScreen->rsDisplayNode_->SetRotation(-90.0f * static_cast<uint32_t>(rotationAfter));
            absScreen->rsDisplayNode_->SetFrame(x, y, w, h);
            absScreen->rsDisplayNode_->SetBounds(x, y, w, h);
            auto transactionProxy = RSTransactionProxy::GetInstance();
            if (transactionProxy != nullptr) {
                transactionProxy->FlushImplicitTransaction();
            }
            absScreen->rotation_ = rotationAfter;
            absScreen->SetOrientation(absScreen->screenRequestedOrientation_);
        }
    }
    NotifyScreenConnected(absScreen->ConvertToScreenInfo());
    NotifyScreenGroupChanged(absScreen->ConvertToScreenInfo(), ScreenGroupChangeEvent::ADD_TO_GROUP);
    if (abstractScreenCallback_ != nullptr) {
        abstractScreenCallback_->onConnect_(absScreen);
    }
}

sptr<AbstractScreen> AbstractScreenController::InitAndGetScreen(ScreenId rsScreenId)
{
    ScreenId dmsScreenId = screenIdManager_.CreateAndGetNewScreenId(rsScreenId);
    RSScreenCapability screenCapability = rsInterface_.GetScreenCapability(rsScreenId);
    WLOGFD("Screen name is %{public}s, phyWidth is %{public}u, phyHeight is %{public}u",
        screenCapability.GetName().c_str(), screenCapability.GetPhyWidth(), screenCapability.GetPhyHeight());

    sptr<AbstractScreen> absScreen =
        new(std::nothrow) AbstractScreen(this, screenCapability.GetName(), dmsScreenId, rsScreenId);
    if (absScreen == nullptr) {
        WLOGFE("new AbstractScreen failed.");
        screenIdManager_.DeleteScreenId(dmsScreenId);
        return nullptr;
    }
    absScreen->SetPhyWidth(screenCapability.GetPhyWidth());
    absScreen->SetPhyHeight(screenCapability.GetPhyHeight());

    if (!InitAbstractScreenModesInfo(absScreen)) {
        screenIdManager_.DeleteScreenId(dmsScreenId);
        WLOGFE("InitAndGetScreen failed.");
        return nullptr;
    }
    dmsScreenMap_.insert(std::make_pair(dmsScreenId, absScreen));
    return absScreen;
}

void AbstractScreenController::ProcessScreenDisconnected(ScreenId rsScreenId)
{
    ScreenId dmsScreenId;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!screenIdManager_.ConvertToDmsScreenId(rsScreenId, dmsScreenId)) {
        WLOGFE("disconnect screen, rsScreenId=%{public}" PRIu64" is not in rs2DmsScreenIdMap_", rsScreenId);
        return;
    }
    WLOGFI("disconnect screen, rsScreenId= %{public}" PRIu64", dmsScreenId= %{public}" PRIu64"",
        rsScreenId, dmsScreenId);
    auto dmsScreenMapIter = dmsScreenMap_.find(dmsScreenId);
    sptr<AbstractScreenGroup> screenGroup;
    if (dmsScreenMapIter != dmsScreenMap_.end()) {
        auto screen = dmsScreenMapIter->second;
        if (abstractScreenCallback_ != nullptr && CheckScreenInScreenGroup(screen)) {
            if (rsScreenId == defaultRsScreenId_ && defaultRsScreenId_ != SCREEN_ID_INVALID) {
                // Disconnect default screen
                RemoveDefaultScreenFromGroupLocked(screen);
                return;
            }
            abstractScreenCallback_->onDisconnect_(screen);
        }
        screenGroup = RemoveFromGroupLocked(screen);
        if (screenGroup != nullptr) {
            NotifyScreenGroupChanged(screen->ConvertToScreenInfo(), ScreenGroupChangeEvent::REMOVE_FROM_GROUP);
        }
        dmsScreenMap_.erase(dmsScreenMapIter);
        NotifyScreenDisconnected(dmsScreenId);
        if (screenGroup != nullptr && screenGroup->combination_ == ScreenCombination::SCREEN_MIRROR &&
            screen->dmsId_ == screenGroup->mirrorScreenId_ && screenGroup->GetChildCount() != 0) {
            auto defaultScreenId = GetDefaultAbstractScreenId();
            std::vector<ScreenId> screens;
            for (auto& screenItem : screenGroup->GetChildren()) {
                if (screenItem->dmsId_ != defaultScreenId) {
                    screens.emplace_back(screenItem->dmsId_);
                }
            }
            MakeMirror(defaultScreenId, screens);
        }
    }
    screenIdManager_.DeleteScreenId(dmsScreenId);
}

bool AbstractScreenController::InitAbstractScreenModesInfo(sptr<AbstractScreen>& absScreen)
{
    std::vector<RSScreenModeInfo> allModes = rsInterface_.GetScreenSupportedModes(absScreen->rsId_);
    if (allModes.size() == 0) {
        WLOGE("supported screen mode is 0, screenId=%{public}" PRIu64"", absScreen->rsId_);
        return false;
    }
    for (const RSScreenModeInfo& rsScreenModeInfo : allModes) {
        sptr<SupportedScreenModes> info = new(std::nothrow) SupportedScreenModes();
        if (info == nullptr) {
            WLOGFE("create SupportedScreenModes failed");
            return false;
        }
        info->id_ = static_cast<uint32_t>(rsScreenModeInfo.GetScreenModeId());
        info->width_ = static_cast<uint32_t>(rsScreenModeInfo.GetScreenWidth());
        info->height_ = static_cast<uint32_t>(rsScreenModeInfo.GetScreenHeight());
        info->refreshRate_ = rsScreenModeInfo.GetScreenRefreshRate();
        absScreen->modes_.push_back(info);
        WLOGD("fill screen idx:%{public}d w/h:%{public}d/%{public}d",
            rsScreenModeInfo.GetScreenModeId(), info->width_, info->height_);
    }
    int32_t activeModeId = rsInterface_.GetScreenActiveMode(absScreen->rsId_).GetScreenModeId();
    WLOGD("fill screen activeModeId:%{public}d", activeModeId);
    if (static_cast<std::size_t>(activeModeId) >= allModes.size()) {
        WLOGE("activeModeId exceed, screenId=%{public}" PRIu64", activeModeId:%{public}d/%{public}ud",
            absScreen->rsId_, activeModeId, static_cast<uint32_t>(allModes.size()));
        return false;
    }
    absScreen->activeIdx_ = activeModeId;
    return true;
}

sptr<AbstractScreenGroup> AbstractScreenController::AddToGroupLocked(sptr<AbstractScreen> newScreen)
{
    sptr<AbstractScreenGroup> res;
    if (dmsScreenGroupMap_.empty()) {
        WLOGFI("connect the first screen");
        res = AddAsFirstScreenLocked(newScreen);
    } else {
        res = AddAsSuccedentScreenLocked(newScreen);
    }
    return res;
}

void AbstractScreenController::RemoveDefaultScreenFromGroupLocked(sptr<AbstractScreen> screen)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (screen == nullptr) {
        return;
    }
    auto groupDmsId = screen->groupDmsId_;
    auto iter = dmsScreenGroupMap_.find(groupDmsId);
    if (iter == dmsScreenGroupMap_.end()) {
        WLOGFE("groupDmsId:%{public}" PRIu64"is not in dmsScreenGroupMap_.", groupDmsId);
        return;
    }
    sptr<AbstractScreenGroup> screenGroup = iter->second;
    if (screenGroup == nullptr) {
        return;
    }
    auto rsScreenId = screen->rsId_;
    bool res = screenGroup->RemoveDefaultScreen(screen);
    if (!res) {
        WLOGFE("RemoveDefaultScreen failed, rsScreenId: %{public}" PRIu64"", rsScreenId);
    }
}

sptr<AbstractScreenGroup> AbstractScreenController::RemoveFromGroupLocked(sptr<AbstractScreen> screen)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto groupDmsId = screen->groupDmsId_;
    auto iter = dmsScreenGroupMap_.find(groupDmsId);
    if (iter == dmsScreenGroupMap_.end()) {
        WLOGFE("groupDmsId:%{public}" PRIu64"is not in dmsScreenGroupMap_.", groupDmsId);
        return nullptr;
    }
    sptr<AbstractScreenGroup> screenGroup = iter->second;
    if (!RemoveChildFromGroup(screen, screenGroup)) {
        WLOGFE("RemoveChildFromGroup failed");
        return nullptr;
    }
    return screenGroup;
}

bool AbstractScreenController::RemoveChildFromGroup(sptr<AbstractScreen> screen, sptr<AbstractScreenGroup> screenGroup)
{
    bool res = screenGroup->RemoveChild(screen);
    if (!res) {
        WLOGFE("remove screen:%{public}" PRIu64" failed from screenGroup:%{public}" PRIu64".",
              screen->dmsId_, screen->groupDmsId_);
        return false;
    }
    if (screenGroup->GetChildCount() == 0) {
        // Group removed, need to do something.
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        dmsScreenGroupMap_.erase(screenGroup->dmsId_);
        dmsScreenMap_.erase(screenGroup->dmsId_);
    }
    return true;
}

bool AbstractScreenController::CheckScreenInScreenGroup(sptr<AbstractScreen> screen) const
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto groupDmsId = screen->groupDmsId_;
    auto iter = dmsScreenGroupMap_.find(groupDmsId);
    if (iter == dmsScreenGroupMap_.end()) {
        WLOGFE("groupDmsId:%{public}" PRIu64"is not in dmsScreenGroupMap_.", groupDmsId);
        return false;
    }
    sptr<AbstractScreenGroup> screenGroup = iter->second;
    return screenGroup->HasChild(screen->dmsId_);
}

sptr<AbstractScreenGroup> AbstractScreenController::AddAsFirstScreenLocked(sptr<AbstractScreen> newScreen)
{
    ScreenId dmsGroupScreenId = screenIdManager_.CreateAndGetNewScreenId(SCREEN_ID_INVALID);
    std::ostringstream buffer;
    buffer<<"ScreenGroup_"<<dmsGroupScreenId;
    std::string name = buffer.str();
    // default ScreenCombination is mirror
    isExpandCombination_ = system::GetParameter("persist.display.expand.enabled", "0") == "1";
    sptr<AbstractScreenGroup> screenGroup;
    if (isExpandCombination_) {
        screenGroup = new(std::nothrow) AbstractScreenGroup(this, dmsGroupScreenId,
            SCREEN_ID_INVALID, name, ScreenCombination::SCREEN_EXPAND);
    } else {
        screenGroup = new(std::nothrow) AbstractScreenGroup(this, dmsGroupScreenId,
            SCREEN_ID_INVALID, name, ScreenCombination::SCREEN_MIRROR);
    }

    if (screenGroup == nullptr) {
        WLOGE("new AbstractScreenGroup failed");
        screenIdManager_.DeleteScreenId(dmsGroupScreenId);
        return nullptr;
    }
    Point point;
    if (!screenGroup->AddChild(newScreen, point)) {
        WLOGE("fail to add screen to group. screen=%{public}" PRIu64"", newScreen->dmsId_);
        screenIdManager_.DeleteScreenId(dmsGroupScreenId);
        return nullptr;
    }
    auto iter = dmsScreenGroupMap_.find(dmsGroupScreenId);
    if (iter != dmsScreenGroupMap_.end()) {
        WLOGE("group screen existed. id=%{public}" PRIu64"", dmsGroupScreenId);
        dmsScreenGroupMap_.erase(iter);
    }
    dmsScreenGroupMap_.insert(std::make_pair(dmsGroupScreenId, screenGroup));
    dmsScreenMap_.insert(std::make_pair(dmsGroupScreenId, screenGroup));
    screenGroup->mirrorScreenId_ = newScreen->dmsId_;
    WLOGI("connect new group screen, screenId: %{public}" PRIu64", screenGroupId: %{public}" PRIu64", "
        "combination:%{public}u", newScreen->dmsId_, dmsGroupScreenId, newScreen->type_);
    return screenGroup;
}

sptr<AbstractScreenGroup> AbstractScreenController::AddAsSuccedentScreenLocked(sptr<AbstractScreen> newScreen)
{
    ScreenId defaultScreenId = GetDefaultAbstractScreenId();
    auto iter = dmsScreenMap_.find(defaultScreenId);
    if (iter == dmsScreenMap_.end()) {
        WLOGE("AddAsSuccedentScreenLocked. defaultScreenId:%{public}" PRIu64" is not in dmsScreenMap_.",
            defaultScreenId);
        return nullptr;
    }
    auto screen = iter->second;
    auto screenGroupIter = dmsScreenGroupMap_.find(screen->groupDmsId_);
    if (screenGroupIter == dmsScreenGroupMap_.end()) {
        WLOGE("AddAsSuccedentScreenLocked. groupDmsId:%{public}" PRIu64" is not in dmsScreenGroupMap_.",
            screen->groupDmsId_);
        return nullptr;
    }
    auto screenGroup = screenGroupIter->second;
    Point point;
    if (screenGroup->combination_ == ScreenCombination::SCREEN_EXPAND) {
        point = {screen->GetActiveScreenMode()->width_, 0};
    }
    screenGroup->AddChild(newScreen, point);
    return screenGroup;
}

ScreenId AbstractScreenController::CreateVirtualScreen(VirtualScreenOption option,
    const sptr<IRemoteObject>& displayManagerAgent)
{
    ScreenId rsId = rsInterface_.CreateVirtualScreen(option.name_, option.width_,
        option.height_, option.surface_, SCREEN_ID_INVALID, option.flags_);
    WLOGFI("id: %{public}" PRIu64"", rsId);
    if (rsId == SCREEN_ID_INVALID) {
        return SCREEN_ID_INVALID;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    ScreenId dmsScreenId = SCREEN_ID_INVALID;
    if (!screenIdManager_.ConvertToDmsScreenId(rsId, dmsScreenId)) {
        dmsScreenId = screenIdManager_.CreateAndGetNewScreenId(rsId);
        auto absScreen = InitVirtualScreen(dmsScreenId, rsId, option);
        if (absScreen == nullptr) {
            screenIdManager_.DeleteScreenId(dmsScreenId);
            return SCREEN_ID_INVALID;
        }
        dmsScreenMap_.insert(std::make_pair(dmsScreenId, absScreen));
        NotifyScreenConnected(absScreen->ConvertToScreenInfo());
        if (deathRecipient_ == nullptr) {
            deathRecipient_ =
                new AgentDeathRecipient([this](const sptr<IRemoteObject>& agent) { OnRemoteDied(agent); });
        }
        auto agIter = screenAgentMap_.find(displayManagerAgent);
        if (agIter == screenAgentMap_.end()) {
            displayManagerAgent->AddDeathRecipient(deathRecipient_);
        }
        screenAgentMap_[displayManagerAgent].emplace_back(dmsScreenId);
    } else {
        WLOGFI("id: %{public}" PRIu64" appears in screenIdManager_. ", rsId);
    }
    return dmsScreenId;
}

sptr<AbstractScreen> AbstractScreenController::InitVirtualScreen(ScreenId dmsScreenId, ScreenId rsId,
    VirtualScreenOption option)
{
    sptr<AbstractScreen> absScreen = new(std::nothrow) AbstractScreen(this, option.name_, dmsScreenId, rsId);
    sptr<SupportedScreenModes> info = new(std::nothrow) SupportedScreenModes();
    if (absScreen == nullptr || info == nullptr) {
        WLOGFI("new AbstractScreen or SupportedScreenModes failed");
        screenIdManager_.DeleteScreenId(dmsScreenId);
        rsInterface_.RemoveVirtualScreen(rsId);
        return nullptr;
    }
    info->width_ = option.width_;
    info->height_ = option.height_;
    auto defaultScreen = GetAbstractScreen(GetDefaultAbstractScreenId());
    if (defaultScreen != nullptr && defaultScreen->GetActiveScreenMode() != nullptr) {
        info->refreshRate_ = defaultScreen->GetActiveScreenMode()->refreshRate_;
    }
    absScreen->modes_.emplace_back(info);
    absScreen->activeIdx_ = 0;
    absScreen->type_ = ScreenType::VIRTUAL;
    absScreen->virtualPixelRatio_ = option.density_;
    return absScreen;
}

DMError AbstractScreenController::DestroyVirtualScreen(ScreenId screenId)
{
    WLOGFI("AbstractScreenController::DestroyVirtualScreen");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId);

    sptr<IDisplayManagerAgent> displayManagerAgent = nullptr;
    bool agentFound = false;
    for (auto &agentIter : screenAgentMap_) {
        for (auto iter = agentIter.second.begin(); iter != agentIter.second.end(); iter++) {
            if (*iter == screenId) {
                iter = agentIter.second.erase(iter);
                agentFound = true;
                break;
            }
        }
        if (agentFound) {
            if (agentIter.first != nullptr && agentIter.second.empty()) {
                agentIter.first->RemoveDeathRecipient(deathRecipient_);
                screenAgentMap_.erase(agentIter.first);
            }
            break;
        }
    }

    if (rsScreenId != SCREEN_ID_INVALID && GetAbstractScreen(screenId) != nullptr) {
        ProcessScreenDisconnected(rsScreenId);
    }
    screenIdManager_.DeleteScreenId(screenId);

    if (rsScreenId == SCREEN_ID_INVALID) {
        WLOGFE("DestroyVirtualScreen: No corresponding rsScreenId");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    rsInterface_.RemoveVirtualScreen(rsScreenId);
    return DMError::DM_OK;
}

DMError AbstractScreenController::SetVirtualScreenSurface(ScreenId screenId, sptr<Surface> surface)
{
    WLOGFI("begin");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    int32_t res = -1;
    ScreenId rsScreenId;
    if (screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
        res = rsInterface_.SetVirtualScreenSurface(rsScreenId, surface);
    }
    if (res != 0) {
        WLOGE("SetVirtualScreenSurface failed in RenderService");
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    return DMError::DM_OK;
}

void AbstractScreenController::SetBuildInDefaultOrientation(Orientation orientation)
{
    if (orientation >= Orientation::BEGIN && orientation <= Orientation::END) {
        buildInDefaultOrientation_ = orientation;
    }
}

DMError AbstractScreenController::SetOrientation(ScreenId screenId, Orientation newOrientation,
    bool isFromWindow, bool withAnimation)
{
    WLOGD("set orientation. screen %{public}" PRIu64" orientation %{public}u", screenId, newOrientation);
    auto screen = GetAbstractScreen(screenId);
    if (screen == nullptr) {
        WLOGFE("fail to set orientation, cannot find screen %{public}" PRIu64"", screenId);
        return DMError::DM_ERROR_NULLPTR;
    }
    if (screen->isScreenGroup_) {
        WLOGE("cannot set orientation to the combination. screen: %{public}" PRIu64"", screenId);
        return DMError::DM_ERROR_NULLPTR;
    }
    if (isFromWindow) {
        if (newOrientation == Orientation::UNSPECIFIED) {
            newOrientation = screen->screenRequestedOrientation_;
            withAnimation = true;
        }
    } else {
        screen->screenRequestedOrientation_ = newOrientation;
    }
    if (screen->orientation_ == newOrientation) {
        WLOGFD("skip setting orientation. screen %{public}" PRIu64" orientation %{public}u", screenId, newOrientation);
        return DMError::DM_OK;
    }
    if (isFromWindow) {
        ScreenRotationController::ProcessOrientationSwitch(newOrientation, withAnimation);
    } else {
        Rotation rotationAfter = screen->CalcRotation(newOrientation);
        SetRotation(screenId, rotationAfter, false);
        screen->rotation_ = rotationAfter;
    }
    if (!screen->SetOrientation(newOrientation)) {
        WLOGE("fail to set rotation, screen %{public}" PRIu64"", screenId);
        return DMError::DM_ERROR_NULLPTR;
    }

    // Notify rotation event to ScreenManager
    NotifyScreenChanged(screen->ConvertToScreenInfo(), ScreenChangeEvent::UPDATE_ORIENTATION);
    // Notify rotation event to AbstractDisplayController
    if (abstractScreenCallback_ != nullptr) {
        if (!withAnimation) {
            abstractScreenCallback_->onChange_(screen, DisplayChangeEvent::UPDATE_ORIENTATION_FROM_WINDOW);
        } else {
            abstractScreenCallback_->onChange_(screen, DisplayChangeEvent::UPDATE_ORIENTATION);
        }
    }
    return DMError::DM_OK;
}

void AbstractScreenController::SetScreenRotateAnimation(
    sptr<AbstractScreen>& screen, ScreenId screenId, Rotation rotationAfter, bool withAnimation)
{
    sptr<SupportedScreenModes> abstractScreenModes = screen->GetActiveScreenMode();
    float w = 0;
    float h = 0;
    float x = 0;
    float y = 0;
    if (abstractScreenModes != nullptr) {
        h = abstractScreenModes->height_;
        w = abstractScreenModes->width_;
    }
    if (!IsVertical(rotationAfter)) {
        std::swap(w, h);
        x = (h - w) / 2; // 2: used to calculate offset to center display node
        y = (w - h) / 2; // 2: used to calculate offset to center display node
    }
    auto displayNode = GetRSDisplayNodeByScreenId(screenId);
    if (displayNode == nullptr) {
        return;
    }
    if (rotationAfter == Rotation::ROTATION_0 && screen->rotation_ == Rotation::ROTATION_270) {
        WLOGFD("[FixOrientation] display rotate with animation");
        // avoid animation 270, 240, 210 ... 30, 0, should play from 90->0
        displayNode->SetRotation(90.f);
    } else if (rotationAfter == Rotation::ROTATION_270 && screen->rotation_ == Rotation::ROTATION_0) {
        WLOGFD("[FixOrientation] display rotate with animation");
        // avoid animation 0, 30, 60 ... 270, should play from 360->270
        displayNode->SetRotation(-360.f);
    }
    if (withAnimation) {
        WLOGFD("[FixOrientation] display rotate with animation %{public}u", rotationAfter);
        std::weak_ptr<RSDisplayNode> weakNode = GetRSDisplayNodeByScreenId(screenId);
        static const RSAnimationTimingProtocol timingProtocol(600); // animation time
        static const RSAnimationTimingCurve curve =
            RSAnimationTimingCurve::CreateCubicCurve(0.2, 0.0, 0.2, 1.0); // animation curve: cubic [0.2, 0.0, 0.2, 1.0]
    #ifdef SOC_PERF_ENABLE
        // Increase frequency to improve windowRotation perf
        // 10027 means "gesture" level that setting duration: 800, lit_cpu_min_freq: 1421000, mid_cpu_min_feq: 1882000
        OHOS::SOCPERF::SocPerfClient::GetInstance().PerfRequest(10027, "");
    #endif
        RSNode::Animate(timingProtocol, curve, [weakNode, x, y, w, h, rotationAfter]() {
            auto displayNode = weakNode.lock();
            if (displayNode == nullptr) {
                WLOGFE("error, cannot get DisplayNode");
                return;
            }
            displayNode->SetRotation(-90.f * static_cast<uint32_t>(rotationAfter)); // 90.f is base degree
            displayNode->SetFrame(x, y, w, h);
            displayNode->SetBounds(x, y, w, h);
        }, []() {
    #ifdef SOC_PERF_ENABLE
            // ClosePerf in finishCallBack
            OHOS::SOCPERF::SocPerfClient::GetInstance().PerfRequestEx(10027, false, "");
    #endif
        });
    } else {
        WLOGFD("[FixOrientation] display rotate without animation %{public}u", rotationAfter);
        displayNode->SetRotation(-90.f * static_cast<uint32_t>(rotationAfter)); // 90.f is base degree
        displayNode->SetFrame(x, y, w, h);
        displayNode->SetBounds(x, y, w, h);
    }
}

void AbstractScreenController::OpenRotationSyncTransaction()
{
     // Before open transaction, it must flush first.
    auto transactionProxy = RSTransactionProxy::GetInstance();
    if (!transactionProxy) {
        return;
    }
    transactionProxy->FlushImplicitTransaction();
    auto syncTransactionController = RSSyncTransactionController::GetInstance();
    if (syncTransactionController) {
        syncTransactionController->OpenSyncTransaction();
    }
}

void AbstractScreenController::CloseRotationSyncTransaction()
{
    auto syncTransactionController = RSSyncTransactionController::GetInstance();
    if (syncTransactionController) {
        syncTransactionController->CloseSyncTransaction();
    }
}

bool AbstractScreenController::SetRotation(ScreenId screenId, Rotation rotationAfter,
    bool isFromWindow, bool withAnimation)
{
    WLOGFI("Enter SetRotation, screenId: %{public}" PRIu64 ", rotation: %{public}u, isFromWindow: %{public}u,"
        "animation: %{public}u", screenId, rotationAfter, isFromWindow, withAnimation);
    auto screen = GetAbstractScreen(screenId);
    if (screen == nullptr) {
        WLOGFE("SetRotation error, cannot get screen with screenId: %{public}" PRIu64, screenId);
        return false;
    }
    if (rotationAfter == screen->rotation_) {
        WLOGFD("rotation not changed. screen %{public}" PRIu64" rotation %{public}u", screenId, rotationAfter);
        return false;
    }
    WLOGFD("set orientation. rotation %{public}u", rotationAfter);
    OpenRotationSyncTransaction();
    SetScreenRotateAnimation(screen, screenId, rotationAfter, withAnimation);
    screen->rotation_ = rotationAfter;
    CloseRotationSyncTransaction();

    NotifyScreenChanged(screen->ConvertToScreenInfo(), ScreenChangeEvent::UPDATE_ROTATION);
    // Notify rotation event to AbstractDisplayController
    if (abstractScreenCallback_ != nullptr) {
        if (!withAnimation) {
            abstractScreenCallback_->onChange_(screen, DisplayChangeEvent::UPDATE_ROTATION_FROM_WINDOW);
        } else {
            abstractScreenCallback_->onChange_(screen, DisplayChangeEvent::UPDATE_ROTATION);
        }
    }
    return true;
}

DMError AbstractScreenController::GetScreenSupportedColorGamuts(ScreenId screenId,
    std::vector<ScreenColorGamut>& colorGamuts)
{
    sptr<AbstractScreen> screen = GetAbstractScreen(screenId);
    if (screen == nullptr) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screen->GetScreenSupportedColorGamuts(colorGamuts);
}

DMError AbstractScreenController::GetScreenColorGamut(ScreenId screenId, ScreenColorGamut& colorGamut)
{
    sptr<AbstractScreen> screen = GetAbstractScreen(screenId);
    if (screen == nullptr) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screen->GetScreenColorGamut(colorGamut);
}

DMError AbstractScreenController::SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx)
{
    sptr<AbstractScreen> screen = GetAbstractScreen(screenId);
    if (screen == nullptr) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screen->SetScreenColorGamut(colorGamutIdx);
}

DMError AbstractScreenController::GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap)
{
    sptr<AbstractScreen> screen = GetAbstractScreen(screenId);
    if (screen == nullptr) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screen->GetScreenGamutMap(gamutMap);
}

DMError AbstractScreenController::SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap)
{
    sptr<AbstractScreen> screen = GetAbstractScreen(screenId);
    if (screen == nullptr) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screen->SetScreenGamutMap(gamutMap);
}

DMError AbstractScreenController::SetScreenColorTransform(ScreenId screenId)
{
    sptr<AbstractScreen> screen = GetAbstractScreen(screenId);
    if (screen == nullptr) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screen->SetScreenColorTransform();
}

DMError AbstractScreenController::SetScreenActiveMode(ScreenId screenId, uint32_t modeId)
{
    WLOGI("SetScreenActiveMode: RsScreenId: %{public}" PRIu64", modeId: %{public}u", screenId, modeId);
    if (screenId == SCREEN_ID_INVALID) {
        WLOGFE("SetScreenActiveMode: invalid screenId");
        return DMError::DM_ERROR_NULLPTR;
    }
    uint32_t usedModeId = 0;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto screen = GetAbstractScreen(screenId);
        if (screen == nullptr) {
            WLOGFE("SetScreenActiveMode: Get AbstractScreen failed");
            return DMError::DM_ERROR_NULLPTR;
        }
        ScreenId rsScreenId = SCREEN_ID_INVALID;
        if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
            WLOGFE("SetScreenActiveMode: No corresponding rsId");
            return DMError::DM_ERROR_NULLPTR;
        }
        rsInterface_.SetScreenActiveMode(rsScreenId, modeId);
        usedModeId = static_cast<uint32_t>(screen->activeIdx_);
        screen->activeIdx_ = static_cast<int32_t>(modeId);
    }
    // add thread to process mode change sync event
    if (usedModeId != modeId) {
        WLOGI("SetScreenActiveMode: modeId: %{public}u ->  %{public}u", usedModeId, modeId);
        auto func = [=]() {
            ProcessScreenModeChanged(screenId);
            return;
        };
        controllerHandler_->PostTask(func, AppExecFwk::EventQueue::Priority::HIGH);
    }
    return DMError::DM_OK;
}

void AbstractScreenController::ProcessScreenModeChanged(ScreenId dmsScreenId)
{
    sptr<AbstractScreen> absScreen = nullptr;
    sptr<AbstractScreenCallback> absScreenCallback = nullptr;
    sptr<SupportedScreenModes> activeScreenMode = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto dmsScreenMapIter = dmsScreenMap_.find(dmsScreenId);
        if (dmsScreenMapIter == dmsScreenMap_.end()) {
            WLOGFE("dmsScreenId=%{public}" PRIu64" is not in dmsScreenMap", dmsScreenId);
            return;
        }
        absScreen = GetAbstractScreen(dmsScreenId);
        if (absScreen == nullptr) {
            WLOGFE("screen is nullptr. dmsScreenId=%{public}" PRIu64"", dmsScreenId);
            return;
        }
        activeScreenMode = absScreen->GetActiveScreenMode();
        if (activeScreenMode == nullptr) {
            WLOGFE("active screen mode is nullptr. dmsScreenId=%{public}" PRIu64"",
                dmsScreenId);
            return;
        }
        absScreenCallback = abstractScreenCallback_;
    }
    uint32_t width = activeScreenMode->width_;
    uint32_t height = activeScreenMode->height_;
    uint32_t refreshRate = activeScreenMode->refreshRate_;
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:ProcessScreenModeChanged(%" PRIu64"),\
        width*height(%u*%u), refreshRate(%u)", dmsScreenId, width, height, refreshRate);
    if (absScreenCallback != nullptr) {
        absScreenCallback->onChange_(absScreen, DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
    }
    NotifyScreenChanged(absScreen->ConvertToScreenInfo(), ScreenChangeEvent::CHANGE_MODE);
}

DMError AbstractScreenController::MakeMirror(ScreenId screenId, std::vector<ScreenId> screens)
{
    WLOGI("MakeMirror, screenId:%{public}" PRIu64"", screenId);
    sptr<AbstractScreen> screen = GetAbstractScreen(screenId);
    if (screen == nullptr || screen->type_ != ScreenType::REAL) {
        WLOGFE("screen is nullptr, or screenType is not real.");
        return DMError::DM_ERROR_NULLPTR;
    }
    WLOGFD("GetAbstractScreenGroup start");
    auto group = GetAbstractScreenGroup(screen->groupDmsId_);
    if (group == nullptr) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        group = AddToGroupLocked(screen);
        if (group == nullptr) {
            WLOGFE("group is nullptr");
            return DMError::DM_ERROR_NULLPTR;
        }
        NotifyScreenGroupChanged(screen->ConvertToScreenInfo(), ScreenGroupChangeEvent::ADD_TO_GROUP);
        if (group != nullptr && abstractScreenCallback_ != nullptr) {
            abstractScreenCallback_->onConnect_(screen);
        }
    }
    WLOGFD("GetAbstractScreenGroup end");
    Point point;
    std::vector<Point> startPoints;
    startPoints.insert(startPoints.begin(), screens.size(), point);
    bool filterMirroredScreen =
        group->combination_ == ScreenCombination::SCREEN_MIRROR && group->mirrorScreenId_ == screen->dmsId_;
    group->mirrorScreenId_ = screen->dmsId_;
    ChangeScreenGroup(group, screens, startPoints, filterMirroredScreen, ScreenCombination::SCREEN_MIRROR);
    WLOGFI("MakeMirror success");
    return DMError::DM_OK;
}

DMError AbstractScreenController::StopScreens(const std::vector<ScreenId>& screenIds, ScreenCombination stopCombination)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (ScreenId screenId : screenIds) {
        WLOGFI("ScreenId: %{public}" PRIu64"", screenId);
        auto screen = GetAbstractScreen(screenId);
        if (screen == nullptr) {
            WLOGFW("screen:%{public}" PRIu64" is nullptr", screenId);
            continue;
        }
        auto iter = dmsScreenGroupMap_.find(screen->groupDmsId_);
        if (iter == dmsScreenGroupMap_.end()) {
            WLOGFW("groupDmsId:%{public}" PRIu64"is not in dmsScreenGroupMap_", screen->groupDmsId_);
            continue;
        }
        sptr<AbstractScreenGroup> screenGroup = iter->second;
        if (screenGroup == nullptr) {
            WLOGFW("screenGroup:%{public}" PRIu64" is nullptr", screen->groupDmsId_);
            continue;
        }
        if (screenGroup->combination_ != stopCombination) {
            WLOGFW("try to stop screen in another combination");
            continue;
        }
        if (screenGroup->combination_ == ScreenCombination::SCREEN_MIRROR &&
            screen->dmsId_ == screenGroup->mirrorScreenId_) {
            WLOGFW("try to stop main mirror screen");
            continue;
        }

        if (abstractScreenCallback_ != nullptr && CheckScreenInScreenGroup(screen)) {
            abstractScreenCallback_->onDisconnect_(screen);
        }
        bool res = RemoveChildFromGroup(screen, screenGroup);
        if (res) {
            NotifyScreenGroupChanged(screen->ConvertToScreenInfo(), ScreenGroupChangeEvent::REMOVE_FROM_GROUP);
        }
    }
    return DMError::DM_OK;
}

void AbstractScreenController::ChangeScreenGroup(sptr<AbstractScreenGroup> group, const std::vector<ScreenId>& screens,
    const std::vector<Point>& startPoints, bool filterScreen, ScreenCombination combination)
{
    std::map<ScreenId, bool> removeChildResMap;
    std::vector<ScreenId> addScreens;
    std::vector<Point> addChildPos;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (uint64_t i = 0; i != screens.size(); i++) {
        ScreenId screenId = screens[i];
        WLOGFI("ScreenId: %{public}" PRIu64"", screenId);
        auto screen = GetAbstractScreen(screenId);
        if (screen == nullptr) {
            WLOGFE("screen:%{public}" PRIu64" is nullptr", screenId);
            continue;
        }
        WLOGFI("Screen->groupDmsId_: %{public}" PRIu64"", screen->groupDmsId_);
        if (filterScreen && screen->groupDmsId_ == group->dmsId_ && group->HasChild(screen->dmsId_)) {
            continue;
        }
        if (abstractScreenCallback_ != nullptr && CheckScreenInScreenGroup(screen)) {
            abstractScreenCallback_->onDisconnect_(screen);
        }
        auto originGroup = RemoveFromGroupLocked(screen);
        addChildPos.emplace_back(startPoints[i]);
        removeChildResMap[screenId] = originGroup != nullptr;
        addScreens.emplace_back(screenId);
    }
    group->combination_ = combination;
    AddScreenToGroup(group, addScreens, addChildPos, removeChildResMap);
}

void AbstractScreenController::AddScreenToGroup(sptr<AbstractScreenGroup> group,
    const std::vector<ScreenId>& addScreens, const std::vector<Point>& addChildPos,
    std::map<ScreenId, bool>& removeChildResMap)
{
    std::vector<sptr<ScreenInfo>> addToGroup;
    std::vector<sptr<ScreenInfo>> removeFromGroup;
    std::vector<sptr<ScreenInfo>> changeGroup;
    for (uint64_t i = 0; i != addScreens.size(); i++) {
        ScreenId screenId = addScreens[i];
        sptr<AbstractScreen> screen = GetAbstractScreen(screenId);
        if (screen == nullptr) {
            continue;
        }
        Point expandPoint = addChildPos[i];
        WLOGFI("screenId: %{public}" PRIu64", Point: %{public}d, %{public}d",
            screen->dmsId_, expandPoint.posX_, expandPoint.posY_);
        bool addChildRes = group->AddChild(screen, expandPoint);
        if (removeChildResMap[screenId] && addChildRes) {
            changeGroup.emplace_back(screen->ConvertToScreenInfo());
            WLOGFD("changeGroup");
        } else if (removeChildResMap[screenId]) {
            WLOGFD("removeChild");
            removeFromGroup.emplace_back(screen->ConvertToScreenInfo());
        } else if (addChildRes) {
            WLOGFD("AddChild");
            addToGroup.emplace_back(screen->ConvertToScreenInfo());
        } else {
            WLOGFD("default, AddChild failed");
        }
        if (abstractScreenCallback_ != nullptr) {
            abstractScreenCallback_->onConnect_(screen);
        }
    }

    NotifyScreenGroupChanged(removeFromGroup, ScreenGroupChangeEvent::REMOVE_FROM_GROUP);
    NotifyScreenGroupChanged(changeGroup, ScreenGroupChangeEvent::CHANGE_GROUP);
    NotifyScreenGroupChanged(addToGroup, ScreenGroupChangeEvent::ADD_TO_GROUP);
}

bool AbstractScreenController::MakeExpand(std::vector<ScreenId> screenIds, std::vector<Point> startPoints)
{
    ScreenId defaultScreenId = GetDefaultAbstractScreenId();
    WLOGI("MakeExpand, defaultScreenId:%{public}" PRIu64"", defaultScreenId);
    auto defaultScreen = GetAbstractScreen(defaultScreenId);
    if (defaultScreen == nullptr) {
        return false;
    }
    auto group = GetAbstractScreenGroup(defaultScreen->groupDmsId_);
    if (group == nullptr) {
        return false;
    }
    bool filterExpandScreen = group->combination_ == ScreenCombination::SCREEN_EXPAND;
    ChangeScreenGroup(group, screenIds, startPoints, filterExpandScreen, ScreenCombination::SCREEN_EXPAND);
    WLOGFI("MakeExpand success");
    return true;
}

void AbstractScreenController::RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens)
{
    if (screens.empty()) {
        return;
    }
    std::vector<sptr<ScreenInfo>> removeFromGroup;
    for (ScreenId screenId : screens) {
        auto screen = GetAbstractScreen(screenId);
        if (screen == nullptr || screen->type_ != ScreenType::VIRTUAL) {
            continue;
        }
        auto originGroup = GetAbstractScreenGroup(screen->groupDmsId_);
        if (originGroup == nullptr) {
            continue;
        }
        if (!originGroup->HasChild(screenId)) {
            continue;
        }
        if (abstractScreenCallback_ != nullptr && CheckScreenInScreenGroup(screen)) {
            abstractScreenCallback_->onDisconnect_(screen);
        }
        RemoveFromGroupLocked(screen);
        removeFromGroup.emplace_back(screen->ConvertToScreenInfo());
    }
    NotifyScreenGroupChanged(removeFromGroup, ScreenGroupChangeEvent::REMOVE_FROM_GROUP);
}

bool AbstractScreenController::OnRemoteDied(const sptr<IRemoteObject>& agent)
{
    if (agent == nullptr) {
        return false;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto agentIter = screenAgentMap_.find(agent);
    if (agentIter != screenAgentMap_.end()) {
        while (screenAgentMap_[agent].size() > 0) {
            auto diedId = screenAgentMap_[agent][0];
            WLOGI("destroy screenId in OnRemoteDied: %{public}" PRIu64"", diedId);
            DMError res = DestroyVirtualScreen(diedId);
            if (res != DMError::DM_OK) {
                WLOGE("destroy failed in OnRemoteDied: %{public}" PRIu64"", diedId);
            }
        }
        screenAgentMap_.erase(agent);
    }
    return true;
}

ScreenId AbstractScreenController::ScreenIdManager::CreateAndGetNewScreenId(ScreenId rsScreenId)
{
    ScreenId dmsScreenId = dmsScreenCount_++;
    if (dms2RsScreenIdMap_.find(dmsScreenId) != dms2RsScreenIdMap_.end()) {
        WLOGFW("dmsScreenId: %{public}" PRIu64" exit in dms2RsScreenIdMap_, warning.", dmsScreenId);
    }
    dms2RsScreenIdMap_[dmsScreenId] = rsScreenId;
    if (rsScreenId == SCREEN_ID_INVALID) {
        return dmsScreenId;
    }
    if (rs2DmsScreenIdMap_.find(rsScreenId) != rs2DmsScreenIdMap_.end()) {
        WLOGFW("rsScreenId: %{public}" PRIu64" exit in rs2DmsScreenIdMap_, warning.", rsScreenId);
    }
    rs2DmsScreenIdMap_[rsScreenId] = dmsScreenId;
    return dmsScreenId;
}

bool AbstractScreenController::ScreenIdManager::DeleteScreenId(ScreenId dmsScreenId)
{
    auto iter = dms2RsScreenIdMap_.find(dmsScreenId);
    if (iter == dms2RsScreenIdMap_.end()) {
        return false;
    }
    ScreenId rsScreenId = iter->second;
    dms2RsScreenIdMap_.erase(dmsScreenId);
    rs2DmsScreenIdMap_.erase(rsScreenId);
    return true;
}

bool AbstractScreenController::ScreenIdManager::HasDmsScreenId(ScreenId dmsScreenId) const
{
    return dms2RsScreenIdMap_.find(dmsScreenId) != dms2RsScreenIdMap_.end();
}

bool AbstractScreenController::ScreenIdManager::HasRsScreenId(ScreenId dmsScreenId) const
{
    return rs2DmsScreenIdMap_.find(dmsScreenId) != rs2DmsScreenIdMap_.end();
}

bool AbstractScreenController::ScreenIdManager::ConvertToRsScreenId(ScreenId dmsScreenId, ScreenId& rsScreenId) const
{
    auto iter = dms2RsScreenIdMap_.find(dmsScreenId);
    if (iter == dms2RsScreenIdMap_.end()) {
        return false;
    }
    rsScreenId = iter->second;
    return true;
}

ScreenId AbstractScreenController::ScreenIdManager::ConvertToRsScreenId(ScreenId dmsScreenId) const
{
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    ConvertToRsScreenId(dmsScreenId, rsScreenId);
    return rsScreenId;
}

bool AbstractScreenController::ScreenIdManager::ConvertToDmsScreenId(ScreenId rsScreenId, ScreenId& dmsScreenId) const
{
    auto iter = rs2DmsScreenIdMap_.find(rsScreenId);
    if (iter == rs2DmsScreenIdMap_.end()) {
        return false;
    }
    dmsScreenId = iter->second;
    return true;
}

ScreenId AbstractScreenController::ScreenIdManager::ConvertToDmsScreenId(ScreenId rsScreenId) const
{
    ScreenId dmsScreenId = SCREEN_ID_INVALID;
    ConvertToDmsScreenId(rsScreenId, dmsScreenId);
    return dmsScreenId;
}

void AbstractScreenController::NotifyScreenConnected(sptr<ScreenInfo> screenInfo) const
{
    if (screenInfo == nullptr) {
        WLOGFE("NotifyScreenConnected error, screenInfo is nullptr.");
        return;
    }
    auto task = [=] {
        WLOGFI("NotifyScreenConnected,  screenId:%{public}" PRIu64"", screenInfo->GetScreenId());
        DisplayManagerAgentController::GetInstance().OnScreenConnect(screenInfo);
    };
    controllerHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
}

void AbstractScreenController::NotifyScreenDisconnected(ScreenId screenId) const
{
    auto task = [=] {
        WLOGFI("NotifyScreenDisconnected,  screenId:%{public}" PRIu64"", screenId);
        DisplayManagerAgentController::GetInstance().OnScreenDisconnect(screenId);
    };
    controllerHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
}

void AbstractScreenController::NotifyScreenChanged(sptr<ScreenInfo> screenInfo, ScreenChangeEvent event) const
{
    if (screenInfo == nullptr) {
        WLOGFE("NotifyScreenChanged error, screenInfo is nullptr.");
        return;
    }
    auto task = [=] {
        WLOGFI("NotifyScreenChanged,  screenId:%{public}" PRIu64"", screenInfo->GetScreenId());
        DisplayManagerAgentController::GetInstance().OnScreenChange(screenInfo, event);
    };
    controllerHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
}

void AbstractScreenController::NotifyScreenGroupChanged(
    const sptr<ScreenInfo>& screenInfo, ScreenGroupChangeEvent event) const
{
    if (screenInfo == nullptr) {
        WLOGFE("screenInfo is nullptr.");
        return;
    }
    std::string trigger = SysCapUtil::GetClientName();
    auto task = [=] {
        WLOGFI("screenId:%{public}" PRIu64", trigger:[%{public}s]", screenInfo->GetScreenId(), trigger.c_str());
        DisplayManagerAgentController::GetInstance().OnScreenGroupChange(trigger, screenInfo, event);
    };
    controllerHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
}

void AbstractScreenController::NotifyScreenGroupChanged(
    const std::vector<sptr<ScreenInfo>>& screenInfo, ScreenGroupChangeEvent event) const
{
    if (screenInfo.empty()) {
        return;
    }
    std::string trigger = SysCapUtil::GetClientName();
    auto task = [=] {
        WLOGFI("trigger:[%{public}s]", trigger.c_str());
        DisplayManagerAgentController::GetInstance().OnScreenGroupChange(trigger, screenInfo, event);
    };
    controllerHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
}

bool AbstractScreenController::SetScreenPowerForAll(ScreenPowerState state,
    PowerStateChangeReason reason, bool needToNotify)
{
    WLOGFI("state:%{public}u, reason:%{public}u", state, reason);
    auto screenIds = GetAllScreenIds();
    if (screenIds.empty()) {
        WLOGFI("no screen info");
        return false;
    }

    ScreenPowerStatus status;
    switch (state) {
        case ScreenPowerState::POWER_ON: {
            status = ScreenPowerStatus::POWER_STATUS_ON;
            powerState_ = ScreenPowerState::POWER_ON;
            break;
        }
        case ScreenPowerState::POWER_OFF: {
            status = ScreenPowerStatus::POWER_STATUS_OFF;
            powerState_ = ScreenPowerState::POWER_OFF;
            break;
        }
        default: {
            WLOGFW("SetScreenPowerStatus state not support");
            return false;
        }
    }

    bool hasSetScreenPower = false;
    for (auto screenId : screenIds) {
        auto screen = GetAbstractScreen(screenId);
        if (screen == nullptr) {
            continue;
        }
        if (screen->type_ != ScreenType::REAL) {
            WLOGD("skip virtual screen %{public}" PRIu64"", screen->dmsId_);
            continue;
        }
        RSInterfaces::GetInstance().SetScreenPowerStatus(screen->rsId_, status);
        WLOGFI("set screen power status. rsscreen %{public}" PRIu64", status %{public}u", screen->rsId_, status);
        hasSetScreenPower = true;
    }
    WLOGFI("SetScreenPowerStatus end");
    if (!hasSetScreenPower) {
        WLOGFI("no real screen");
        return false;
    }
    if (needToNotify) {
        return DisplayManagerAgentController::GetInstance().NotifyDisplayPowerEvent(
            state == ScreenPowerState::POWER_ON ? DisplayPowerEvent::DISPLAY_ON :
            DisplayPowerEvent::DISPLAY_OFF, EventStatus::END);
    }
    return true;
}

ScreenPowerState AbstractScreenController::GetScreenPower(ScreenId dmsScreenId) const
{
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (dmsScreenMap_.find(dmsScreenId) == dmsScreenMap_.end()) {
            WLOGFE("cannot find screen %{public}" PRIu64"", dmsScreenId);
            return ScreenPowerState::INVALID_STATE;
        }
    }

    ScreenId rsId = ConvertToRsScreenId(dmsScreenId);
    auto state = static_cast<ScreenPowerState>(RSInterfaces::GetInstance().GetScreenPowerStatus(rsId));
    WLOGFI("GetScreenPower:%{public}u, rsscreen:%{public}" PRIu64".", state, rsId);
    return state;
}

DMError AbstractScreenController::SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio)
{
    WLOGD("set virtual pixel ratio. screen %{public}" PRIu64" virtualPixelRatio %{public}f",
        screenId, virtualPixelRatio);
    auto screen = GetAbstractScreen(screenId);
    if (screen == nullptr) {
        WLOGFE("fail to set virtual pixel ratio, cannot find screen %{public}" PRIu64"", screenId);
        return DMError::DM_ERROR_NULLPTR;
    }
    if (screen->isScreenGroup_) {
        WLOGE("cannot set virtual pixel ratio to the combination. screen: %{public}" PRIu64"", screenId);
        return DMError::DM_ERROR_NULLPTR;
    }
    if (fabs(screen->virtualPixelRatio_ - virtualPixelRatio) < 1e-6) {
        WLOGE("The density is equivalent to the original value, no update operation is required, aborted.");
        return DMError::DM_OK;
    }
    screen->SetVirtualPixelRatio(virtualPixelRatio);
    // Notify rotation event to AbstractDisplayController
    if (abstractScreenCallback_ != nullptr) {
        abstractScreenCallback_->onChange_(screen, DisplayChangeEvent::DISPLAY_VIRTUAL_PIXEL_RATIO_CHANGED);
    }
    NotifyScreenChanged(screen->ConvertToScreenInfo(), ScreenChangeEvent::VIRTUAL_PIXEL_RATIO_CHANGED);
    return DMError::DM_OK;
}
} // namespace OHOS::Rosen
