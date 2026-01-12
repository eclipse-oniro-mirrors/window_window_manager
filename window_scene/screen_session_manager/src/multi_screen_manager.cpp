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

#include "multi_screen_manager.h"
#include "multi_screen_mode_change_manager.h"
#include "multi_screen_power_change_manager.h"
#include "screen_power_utils.h"
#include "screen_scene_config.h"
#ifdef RES_SCHED_ENABLE
#include "res_sched_client.h"
#include "res_type.h"
#endif
#include "dms_global_mutex.h"

namespace OHOS::Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(MultiScreenManager)
namespace {
const std::string SCREEN_EXTEND = "extend";
const std::string SCREEN_MIRROR = "mirror";
const std::string SCREEN_OUTER_ON = "on";
const std::string SCREEN_OUTER_OFF = "off";
const std::string MULTI_SCREEN_EXIT_STR = "exit";
const std::string MULTI_SCREEN_ENTER_STR = "enter";
const std::string CUSTOM_SCB_SCREEN_NAME = "CustomScbScreen";
const std::string CELIA_VIEW_NAME = "CeliaView";
constexpr int32_t MULTI_SCREEN_EXIT = 0;
constexpr int32_t MULTI_SCREEN_ENTER = 1;
constexpr uint32_t SCREEN_CONNECT_TIMEOUT = 500;
}
MultiScreenManager::MultiScreenManager()
{
    TLOGI(WmsLogTag::DMS, "init");
    lastScreenMode_ = std::make_pair(SCREEN_ID_INVALID, MultiScreenMode::SCREEN_MIRROR);
}

MultiScreenManager::~MultiScreenManager()
{
    TLOGI(WmsLogTag::DMS, "destructor");
}

void MultiScreenManager::FilterPhysicalAndVirtualScreen(const std::vector<ScreenId>& allScreenIds,
    std::vector<ScreenId>& physicalScreenIds, std::vector<ScreenId>& virtualScreenIds)
{
    TLOGW(WmsLogTag::DMS, "enter allScreen size: %{public}u",
        static_cast<uint32_t>(allScreenIds.size()));
    sptr<ScreenSession> defaultSession = ScreenSessionManager::GetInstance().GetDefaultScreenSession();
    if (defaultSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail to get defaultSession");
        return;
    }
    ScreenId defaultScreenId = defaultSession->GetScreenId();
    for (ScreenId screenId : allScreenIds) {
        auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
        if (screenSession == nullptr) {
            continue;
        }
        if (screenSession->GetScreenId() == defaultScreenId) {
            continue;
        }
        if (screenSession->GetMirrorScreenType() == MirrorScreenType::PHYSICAL_MIRROR) {
            physicalScreenIds.emplace_back(screenId);
        } else if (screenSession->GetMirrorScreenType() == MirrorScreenType::VIRTUAL_MIRROR) {
            virtualScreenIds.emplace_back(screenId);
        } else {
            TLOGW(WmsLogTag::DMS, "mirror screen type error");
        }
    }
    TLOGW(WmsLogTag::DMS, "end");
}

DMError MultiScreenManager::VirtualScreenMirrorSwitch(const ScreenId mainScreenId,
    const std::vector<ScreenId>& screenIds, DMRect mainScreenRegion, ScreenId& screenGroupId,
    const RotationOption& rotationOption, bool forceMirror)
{
    TLOGW(WmsLogTag::DMS, "enter size: %{public}u",
        static_cast<uint32_t>(screenIds.size()));
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:VirtualScreenMirrorSwitch start");
    auto mainScreen = ScreenSessionManager::GetInstance().GetScreenSession(mainScreenId);
    if (mainScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen session null fail mainScreenId: %{public}" PRIu64, mainScreenId);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    DMError ret = ScreenSessionManager::GetInstance().SetMirror(mainScreenId, screenIds, mainScreenRegion,
        rotationOption, forceMirror);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "error: %{public}d", ret);
        return ret;
    }
    if (ScreenSessionManager::GetInstance().GetAbstractScreenGroup(mainScreen->groupSmsId_) == nullptr) {
        TLOGE(WmsLogTag::DMS, "get screen group failed main screenId: %{public}" PRIu64, mainScreenId);
        return DMError::DM_ERROR_NULLPTR;
    }
    screenGroupId = mainScreen->groupSmsId_;
    ScreenSessionManager::GetInstance().HandleCastVirtualScreenMirrorRegion();
    TLOGW(WmsLogTag::DMS, "end");
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:VirtualScreenMirrorSwitch end");
    return ret;
}

DMError MultiScreenManager::PhysicalScreenMirrorSwitch(const ScreenId mainScreenId,
    const std::vector<ScreenId>& screenIds, DMRect mirrorRegion, const RotationOption& rotationOption,
    bool forceMirror)
{
    sptr<ScreenSession> defaultSession = ScreenSessionManager::GetInstance().GetScreenSession(mainScreenId);
    if (defaultSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail to get defaultSession");
        return DMError::DM_ERROR_NULLPTR;
    }
    TLOGW(WmsLogTag::DMS, "enter physical screen switch to mirror screen size: %{public}u",
        static_cast<uint32_t>(screenIds.size()));
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:PhysicalScreenMirrorSwitch start");
    NodeId nodeId = defaultSession->GetDisplayNode() == nullptr ? 0 : defaultSession->GetDisplayNode()->GetId();
    for (ScreenId physicalScreenId : screenIds) {
        auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(physicalScreenId);
        if (screenSession == nullptr) {
            continue;
        }
        TLOGW(WmsLogTag::DMS, "switch to mirror physical ScreenId: %{public}" PRIu64, physicalScreenId);
        if (ScreenSessionManager::GetInstance().HasSameScreenCastInfo(physicalScreenId, mainScreenId,
            ScreenCombination::SCREEN_MIRROR) && !forceMirror) {
            if (mirrorRegion != screenSession->GetMirrorScreenRegion().second) {
                screenSession->SetMirrorScreenRegion(defaultSession->GetRSScreenId(), mirrorRegion);
                screenSession->SetIsPhysicalMirrorSwitch(true);
                screenSession->EnableMirrorScreenRegion();
            }
            TLOGW(WmsLogTag::DMS, "already mirror and get a same region.");
            return DMError::DM_OK;
        }
        RSDisplayNodeConfig config = { screenSession->rsId_, true, nodeId, true};
        if (rotationOption.needSetRotation_) {
            config.mirrorSourceRotation = static_cast<uint32_t>(rotationOption.rotation_);
        }
        screenSession->ReuseDisplayNode(config);
        screenSession->SetMirrorScreenRegion(defaultSession->GetRSScreenId(), mirrorRegion);
        screenSession->SetIsPhysicalMirrorSwitch(true);
        screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
        ScreenSessionManager::GetInstance().SetScreenCastInfo(
            physicalScreenId, mainScreenId, ScreenCombination::SCREEN_MIRROR);
        ScreenSessionManager::GetInstance().NotifyScreenChanged(
            screenSession->ConvertToScreenInfo(), ScreenChangeEvent::SCREEN_SOURCE_MODE_CHANGE);
        ScreenSessionManager::GetInstance().NotifyDisplayChanged(
            screenSession->ConvertToDisplayInfo(), DisplayChangeEvent::SOURCE_MODE_CHANGED);
    }
    TLOGW(WmsLogTag::DMS, "physical screen switch to mirror end");
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:PhysicalScreenMirrorSwitch end");
    return DMError::DM_OK;
}

DMError MultiScreenManager::PhysicalScreenUniqueSwitch(const std::vector<ScreenId>& screenIds)
{
    TLOGW(WmsLogTag::DMS, "enter screen size: %{public}u",
        static_cast<uint32_t>(screenIds.size()));
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:PhysicalScreenUniqueSwitch start");
    for (ScreenId physicalScreenId : screenIds) {
        auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(physicalScreenId);
        if (screenSession == nullptr) {
            continue;
        }
        TLOGW(WmsLogTag::DMS, "switch to unique physical ScreenId: %{public}" PRIu64, physicalScreenId);
        RSDisplayNodeConfig config = { screenSession->rsId_, false, INVALID_NODEID };
        screenSession->ReuseDisplayNode(config);
        screenSession->SetVirtualPixelRatio(screenSession->GetScreenProperty().GetDefaultDensity());
        {
            std::unique_lock<std::mutex> lock(uniqueScreenMutex_);
            if (screenSession->GetInnerName() == CUSTOM_SCB_SCREEN_NAME ||
                screenSession->GetName() == CELIA_VIEW_NAME) {
                    uniqueScreenTimeoutMap_.insert_or_assign(physicalScreenId, false);
                }
        }
        ScreenSessionManager::GetInstance().OnVirtualScreenChange(physicalScreenId, ScreenEvent::CONNECTED);
        ScreenSessionManager::GetInstance().RemoveScreenCastInfo(physicalScreenId);
        BlockScreenConnect(screenSession, physicalScreenId);
        ScreenSessionManager::GetInstance().NotifyScreenChanged(
            screenSession->ConvertToScreenInfo(), ScreenChangeEvent::SCREEN_SOURCE_MODE_CHANGE);
        ScreenSessionManager::GetInstance().NotifyDisplayChanged(
            screenSession->ConvertToDisplayInfo(), DisplayChangeEvent::SOURCE_MODE_CHANGED);
    }
    TLOGW(WmsLogTag::DMS, "end");
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:PhysicalScreenUniqueSwitch end");
    return DMError::DM_OK;
}

DMError MultiScreenManager::VirtualScreenUniqueSwitch(sptr<ScreenSession> screenSession,
    const std::vector<ScreenId>& screenIds, const UniqueScreenRotationOptions& rotationOptions)
{
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:VirtualScreenUniqueSwitch start");
    TLOGW(WmsLogTag::DMS, "start size: %{public}u",
        static_cast<uint32_t>(screenIds.size()));
    auto group = ScreenSessionManager::GetInstance().GetAbstractScreenGroup(screenSession->groupSmsId_);
    if (group == nullptr) {
        group = ScreenSessionManager::GetInstance().AddToGroupLocked(screenSession, true);
        if (group == nullptr) {
            TLOGE(WmsLogTag::DMS, "group is nullptr");
            return DMError::DM_ERROR_NULLPTR;
        }
        ScreenSessionManager::GetInstance().NotifyScreenGroupChanged(screenSession->ConvertToScreenInfo(),
            ScreenGroupChangeEvent::ADD_TO_GROUP);
    }
    Point point;
    std::vector<Point> startPoints;
    startPoints.insert(startPoints.begin(), screenIds.size(), point);
    ScreenSessionManager::GetInstance().ChangeScreenGroup(group, screenIds, startPoints,
        true, ScreenCombination::SCREEN_UNIQUE);

    for (ScreenId uniqueScreenId : screenIds) {
        auto uniqueScreen = ScreenSessionManager::GetInstance().GetScreenSession(uniqueScreenId);
        if (uniqueScreen != nullptr) {
            uniqueScreen->SetScreenCombination(ScreenCombination::SCREEN_UNIQUE);
            ScreenSessionManager::GetInstance().NotifyDisplayChanged(uniqueScreen->ConvertToDisplayInfo(),
                DisplayChangeEvent::SOURCE_MODE_CHANGED);
            ScreenSessionManager::GetInstance().NotifyScreenChanged(uniqueScreen->ConvertToScreenInfo(),
                ScreenChangeEvent::SCREEN_SWITCH_CHANGE);
        }
        ScreenSessionManager::GetInstance().RemoveScreenCastInfo(uniqueScreenId);
        {
            std::unique_lock<std::mutex> lock(uniqueScreenMutex_);
            if (uniqueScreen != nullptr && (uniqueScreen->GetInnerName() == CUSTOM_SCB_SCREEN_NAME ||
                uniqueScreen->GetName() == CELIA_VIEW_NAME)) {
                    uniqueScreenTimeoutMap_.insert_or_assign(uniqueScreenId, false);
                }
        }
        // virtual screen create callback to notify scb
        ScreenSessionManager::GetInstance().OnVirtualScreenChange(uniqueScreenId, ScreenEvent::CONNECTED,
            rotationOptions);
        BlockScreenConnect(uniqueScreen, uniqueScreenId);
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:VirtualScreenUniqueSwitch end");
    TLOGW(WmsLogTag::DMS, "to unique and notify scb end");
    return DMError::DM_OK;
}

void MultiScreenManager::BlockScreenConnect(sptr<ScreenSession>& screenSession, ScreenId screenId)
{
    std::unique_lock<std::mutex> lock(uniqueScreenMutex_);
    if (screenSession != nullptr && (screenSession->GetInnerName() == CUSTOM_SCB_SCREEN_NAME ||
        screenSession->GetName() == CELIA_VIEW_NAME)) {
        auto func = [this, screenId] {
            return uniqueScreenTimeoutMap_[screenId];
        };
        if (!DmUtils::safe_wait_for(uniqueScreenCV_, lock, std::chrono::milliseconds(SCREEN_CONNECT_TIMEOUT), func)) {
            TLOGE(WmsLogTag::DMS, "wait for screen connect timeout, screenId:%{public}" PRIu64,
                screenId);
        }
        uniqueScreenTimeoutMap_.erase(screenId);
    }
}

void MultiScreenManager::NotifyScreenConnectCompletion(ScreenId screenId)
{
    std::unique_lock<std::mutex> lock(uniqueScreenMutex_);
    TLOGI(WmsLogTag::DMS, "ENTER, screenId:%{public}" PRIu64, screenId);
    auto it = uniqueScreenTimeoutMap_.find(screenId);
    if (it != uniqueScreenTimeoutMap_.end()) {
        if (!(it->second)) {
            uniqueScreenTimeoutMap_[screenId] = true;
            uniqueScreenCV_.notify_all();
        }
    }
}

static void AddUniqueScreenDisplayId(std::vector<DisplayId>& displayIds,
    std::vector<ScreenId>& screenIds, DMError& switchStatus)
{
    if (switchStatus == DMError::DM_OK) {
        for (auto screenId : screenIds) {
            displayIds.emplace_back(static_cast<uint64_t>(screenId));
        }
    }
}

DMError MultiScreenManager::UniqueSwitch(const std::vector<ScreenId>& screenIds, std::vector<DisplayId>& displayIds,
    const UniqueScreenRotationOptions& rotationOptions)
{
    DMError switchStatus = DMError::DM_OK;
    std::vector<ScreenId> virtualScreenIds;
    std::vector<ScreenId> physicalScreenIds;
    if (screenIds.empty()) {
        TLOGW(WmsLogTag::DMS, "mirror to screen size empty");
        return switchStatus;
    }
    TLOGW(WmsLogTag::DMS, "enter mirror to screen size: %{public}u",
        static_cast<uint32_t>(screenIds.size()));
    FilterPhysicalAndVirtualScreen(screenIds, physicalScreenIds, virtualScreenIds);
    TLOGD(WmsLogTag::DMS,
        "ScreenSessionManager processing with parameters, isRotationLocked: %{public}d, rotation: %{public}d",
        rotationOptions.isRotationLocked_, rotationOptions.rotation_);
    if (!virtualScreenIds.empty()) {
        switchStatus = ScreenSessionManager::GetInstance().VirtualScreenUniqueSwitch(virtualScreenIds, rotationOptions);
        TLOGW(WmsLogTag::DMS, "virtual screen switch to unique result: %{public}d", switchStatus);
        AddUniqueScreenDisplayId(displayIds, virtualScreenIds, switchStatus);
    }
    if (!physicalScreenIds.empty()) {
        switchStatus = PhysicalScreenUniqueSwitch(physicalScreenIds);
        if (switchStatus == DMError::DM_OK) {
            for (auto screenId : physicalScreenIds) {
                ScreenSessionManager::GetInstance().UnregisterSettingWireCastObserver(screenId);
            }
        }
        TLOGW(WmsLogTag::DMS, "physical screen switch to unique result: %{public}d", switchStatus);
        AddUniqueScreenDisplayId(displayIds, physicalScreenIds, switchStatus);
    }
    if (displayIds.empty()) {
        switchStatus = DMError::DM_ERROR_INVALID_PARAM;
    }
    TLOGW(WmsLogTag::DMS, "mirror switch to unique end");
    return switchStatus;
}

DMError MultiScreenManager::MirrorSwitch(const ScreenId mainScreenId, const std::vector<ScreenId>& screenIds,
    DMRect mainScreenRegion, ScreenId& screenGroupId, const RotationOption& rotationOption, bool forceMirror)
{
    DMError switchStatus = DMError::DM_OK;
    std::vector<ScreenId> virtualScreenIds;
    std::vector<ScreenId> physicalScreenIds;
    if (screenIds.empty()) {
        TLOGW(WmsLogTag::DMS, "screen size empty");
        return switchStatus;
    }
    TLOGW(WmsLogTag::DMS, "enter screen size: %{public}u", static_cast<uint32_t>(screenIds.size()));
    FilterPhysicalAndVirtualScreen(screenIds, physicalScreenIds, virtualScreenIds);

    if (!virtualScreenIds.empty()) {
        switchStatus = VirtualScreenMirrorSwitch(mainScreenId, virtualScreenIds, mainScreenRegion,
            screenGroupId, rotationOption, forceMirror);
        TLOGW(WmsLogTag::DMS, "virtual screen switch to mirror result: %{public}d", switchStatus);
    }
    if (!physicalScreenIds.empty()) {
        screenGroupId = 1;
        switchStatus =
            PhysicalScreenMirrorSwitch(mainScreenId, physicalScreenIds, mainScreenRegion, rotationOption, forceMirror);
        if (switchStatus == DMError::DM_OK) {
            for (auto screenId : physicalScreenIds) {
                auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
                ScreenSessionManager::GetInstance().RegisterSettingWireCastObserver(screenSession);
            }
        }
        TLOGW(WmsLogTag::DMS, "physical screen switch to mirror result: %{public}d", switchStatus);
    }
    TLOGW(WmsLogTag::DMS, "end switchStatus: %{public}d", switchStatus);
    return switchStatus;
}

void MultiScreenManager::MultiScreenModeChange(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession,
    const std::string& operateType)
{
    TLOGW(WmsLogTag::DMS, "enter operateType=%{public}s", operateType.c_str());
    if (firstSession == nullptr || secondarySession == nullptr) {
        TLOGE(WmsLogTag::DMS, "params null.");
        return;
    }
    std::ostringstream oss;
    oss << "multiScreen Operate: " << operateType
        << ", firstSession screenId: " << firstSession->GetScreenId()
        << ", rsId: " << firstSession->GetRSScreenId()
        << ", combination: " << static_cast<int32_t>(firstSession->GetScreenCombination())
        << ", secondarySession screenId: " << secondarySession->GetScreenId()
        << ", rsId: " << secondarySession->GetRSScreenId()
        << ", combination: " << static_cast<int32_t>(secondarySession->GetScreenCombination());
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    if (operateType == SCREEN_OUTER_OFF) {
        MultiScreenPowerChangeManager::GetInstance().OnMultiScreenPowerChangeRequest(firstSession, secondarySession,
            MultiScreenPowerSwitchType::SCREEN_SWITCH_OFF);
    } else if (operateType == SCREEN_OUTER_ON) {
        MultiScreenPowerChangeManager::GetInstance().OnMultiScreenPowerChangeRequest(firstSession, secondarySession,
            MultiScreenPowerSwitchType::SCREEN_SWITCH_ON);
    } else if (operateType == SCREEN_EXTEND || operateType == SCREEN_MIRROR) {
        MultiScreenModeChangeManager::GetInstance().OnMultiScreenModeChangeRequest(firstSession,
            secondarySession, operateType);
    } else {
        TLOGW(WmsLogTag::DMS, "multi screen operate type change failed.");
    }
    ScreenSessionManager::GetInstance().HandleResolutionEffectChange();
}

void MultiScreenManager::DoFirstMainChangeExtend(sptr<IScreenSessionManagerClient> scbClient,
    sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession)
{
    TLOGW(WmsLogTag::DMS, "exec switch extend");
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    RSDisplayNodeConfig config = { secondarySession->screenId_, false, INVALID_NODEID};
    secondarySession->ReuseDisplayNode(config);
    secondarySession->SetIsExtend(true);
    firstSession->SetIsExtend(false);
    SessionOption secondaryOption = ScreenSessionManager::GetInstance().GetSessionOption(secondarySession);
    scbClient->OnScreenConnectionChanged(secondaryOption, ScreenEvent::CONNECTED);
    ScreenSessionManager::GetInstance().NotifyScreenChanged(
        secondarySession->ConvertToScreenInfo(), ScreenChangeEvent::SCREEN_SOURCE_MODE_CHANGE);
    TLOGW(WmsLogTag::DMS, "exec switch mirror to extend 4/6 end");
}

void MultiScreenManager::DoFirstMainChangeMirror(sptr<IScreenSessionManagerClient> scbClient,
    sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession)
{
    /* move second screen windows to first screen then set second screen to mirror */
    /* create mirror */
    SessionOption secondaryOption = ScreenSessionManager::GetInstance().GetSessionOption(secondarySession);
    scbClient->OnScreenConnectionChanged(secondaryOption, ScreenEvent::DISCONNECTED);
    /* create first screen mirror */
    NodeId nodeId = firstSession->GetDisplayNode() == nullptr ? 0 : firstSession->GetDisplayNode()->GetId();
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    secondarySession->SetIsExtend(true);
    firstSession->SetIsExtend(false);
    RSDisplayNodeConfig config = { secondarySession->screenId_, true, nodeId };
    secondarySession->ReuseDisplayNode(config);
    ScreenSessionManager::GetInstance().NotifyScreenChanged(
        secondarySession->ConvertToScreenInfo(), ScreenChangeEvent::SCREEN_SOURCE_MODE_CHANGE);
    TLOGW(WmsLogTag::DMS, "exec switch mirror 12/14 end");
}

void MultiScreenManager::DoFirstMainChange(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession,
    const std::string& operateType)
{
    TLOGW(WmsLogTag::DMS, "enter");
    sptr<IScreenSessionManagerClient> scbClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (scbClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "scbClient null");
        return;
    }
    firstSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    ScreenCombination secondaryCombination = secondarySession->GetScreenCombination();
    TLOGW(WmsLogTag::DMS, "current secondary screen mode:%{public}d", secondaryCombination);
    if (operateType == SCREEN_EXTEND) {
        TLOGW(WmsLogTag::DMS, "exec switch extend");
        if (secondaryCombination == ScreenCombination::SCREEN_MIRROR) {
            // mirror to extend 4,6
            DoFirstMainChangeExtend(scbClient, firstSession, secondarySession);
        } else {
            TLOGW(WmsLogTag::DMS, "already extend no need to change");
        }
    } else if (operateType == SCREEN_MIRROR) {
        TLOGW(WmsLogTag::DMS, "exec switch mirror");
        if (secondaryCombination == ScreenCombination::SCREEN_EXTEND) {
            // mirror to extend 12,14
            DoFirstMainChangeMirror(scbClient, firstSession, secondarySession);
        } else {
            TLOGE(WmsLogTag::DMS, "already mirror no need to change");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "param error!");
    }
}

void MultiScreenManager::DoFirstMirrorChangeExtend(sptr<IScreenSessionManagerClient> scbClient,
    sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession)
{
    /* change secondarySession to extend */
    TLOGW(WmsLogTag::DMS, "exec switch extend");
    secondarySession->SetIsExtend(true);
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);

    /* change firstSession from mirror to main */
    firstSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    RSDisplayNodeConfig config = { firstSession->screenId_ };
    firstSession->SetIsExtend(false);
    firstSession->ReuseDisplayNode(config);
    ScreenSessionManager::GetInstance().SetDefaultScreenId(firstSession->GetScreenId());
    SessionOption firstOption = ScreenSessionManager::GetInstance().GetSessionOption(firstSession);
    scbClient->OnScreenConnectionChanged(firstOption, ScreenEvent::CONNECTED);
    /* move secondarySession windows to firstSession */
    TLOGW(WmsLogTag::DMS, "mainScreenId:%{public}" PRIu64", extendScreenId=%{public}" PRIu64,
        firstSession->GetScreenId(), secondarySession->GetScreenId());
    secondarySession->ScreenExtendChange(firstSession->GetScreenId(), secondarySession->GetScreenId());
    TLOGW(WmsLogTag::DMS, "exec switch extend 1/7 end");
}

void MultiScreenManager::DoFirstMirrorChangeMirror(sptr<IScreenSessionManagerClient> scbClient,
    sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession)
{
    /* change firstSession from to mirror */
    TLOGW(WmsLogTag::DMS, "exec switch mirror");
    firstSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    RSDisplayNodeConfig config = { firstSession->screenId_ };
    firstSession->SetIsExtend(false);
    firstSession->ReuseDisplayNode(config);
    ScreenSessionManager::GetInstance().SetDefaultScreenId(firstSession->GetScreenId());
    SessionOption firstOption = ScreenSessionManager::GetInstance().GetSessionOption(firstSession);
    scbClient->OnScreenConnectionChanged(firstOption, ScreenEvent::CONNECTED);

    /* create mirror */
    SessionOption secondaryOption = ScreenSessionManager::GetInstance().GetSessionOption(secondarySession);
    scbClient->OnScreenConnectionChanged(secondaryOption, ScreenEvent::DISCONNECTED);
    /* create inner screen's mirror */
    NodeId nodeId = firstSession->GetDisplayNode() == nullptr ? 0 : firstSession->GetDisplayNode()->GetId();
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    secondarySession->SetIsExtend(true);
    config = {secondarySession->screenId_, true, nodeId };
    secondarySession->ReuseDisplayNode(config);
    TLOGW(WmsLogTag::DMS, "exec switch mirror 2/3/5/8 end");
}

void MultiScreenManager::DoFirstMirrorChange(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession,
    const std::string& operateType)
{
    TLOGW(WmsLogTag::DMS, "enter");
    sptr<IScreenSessionManagerClient> scbClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (scbClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "scbClient null");
        return;
    }
    if (operateType == SCREEN_EXTEND) {
        // 1, 7
        DoFirstMirrorChangeExtend(scbClient, firstSession, secondarySession);
    } else if (operateType == SCREEN_MIRROR) {
        // 2 3 5 8
        DoFirstMirrorChangeMirror(scbClient, firstSession, secondarySession);
    } else {
        TLOGE(WmsLogTag::DMS, "param error!");
    }
}

void MultiScreenManager::DoFirstExtendChangeExtend(sptr<ScreenSession> firstSession,
    sptr<ScreenSession> secondarySession)
{
    /* set firstSession main */
    TLOGW(WmsLogTag::DMS, "exec switch extend");
    firstSession->SetIsExtend(false);
    firstSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    ScreenSessionManager::GetInstance().SetDefaultScreenId(firstSession->GetScreenId());
    /* set secondarySession extend */
    secondarySession->SetIsExtend(true);
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    TLOGW(WmsLogTag::DMS, "mainScreenId:%{public}" PRIu64", extendScreenId=%{public}" PRIu64,
        firstSession->GetScreenId(), secondarySession->GetScreenId());
    firstSession->ScreenExtendChange(firstSession->GetScreenId(), secondarySession->GetScreenId());
    TLOGW(WmsLogTag::DMS, "exec switch extend 9/11/13/15 end");
    /* change main and extend screens's windows */
}

void MultiScreenManager::DoFirstExtendChangeMirror(sptr<ScreenSession> firstSession,
    sptr<ScreenSession> secondarySession)
{
    sptr<IScreenSessionManagerClient> scbClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (scbClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "scbClient null");
        return;
    }
    /* set firstSession main screen */
    TLOGW(WmsLogTag::DMS, "exec switch mirror");
    firstSession->SetIsExtend(false);
    firstSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    ScreenSessionManager::GetInstance().SetDefaultScreenId(firstSession->GetScreenId());
    TLOGW(WmsLogTag::DMS, "mainScreenId:%{public}" PRIu64", extendScreenId=%{public}" PRIu64,
        firstSession->GetScreenId(), secondarySession->GetScreenId());
    firstSession->ScreenExtendChange(firstSession->GetScreenId(), secondarySession->GetScreenId());
    /* move secondarySession windows to firstSession => join two screens window to firstSession */
    /* create mirror */
    SessionOption secondaryOption = ScreenSessionManager::GetInstance().GetSessionOption(secondarySession);
    scbClient->OnScreenConnectionChanged(secondaryOption, ScreenEvent::DISCONNECTED);
    /* create inner screen's mirror node */
    NodeId nodeId = firstSession->GetDisplayNode() == nullptr ? 0 : firstSession->GetDisplayNode()->GetId();
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    secondarySession->SetIsExtend(true);
    RSDisplayNodeConfig config = { secondarySession->screenId_, true, nodeId };
    secondarySession->ReuseDisplayNode(config);
    TLOGW(WmsLogTag::DMS, "exec switch mirror 10/16 end");
}

void MultiScreenManager::DoFirstExtendChange(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession,
    const std::string& operateType)
{
    TLOGW(WmsLogTag::DMS, "enter");
    if (operateType == SCREEN_EXTEND) {
        // 9 11 13 15
        DoFirstExtendChangeExtend(firstSession, secondarySession);
    } else if (operateType == SCREEN_MIRROR) {
        // 10 16
        DoFirstExtendChangeMirror(firstSession, secondarySession);
    } else {
        TLOGE(WmsLogTag::DMS, "param error!");
    }
}

void MultiScreenManager::SetLastScreenMode(ScreenId mainScreenId, MultiScreenMode secondaryScreenMode)
{
    lastScreenMode_.first = mainScreenId;
    lastScreenMode_.second = secondaryScreenMode;
    TLOGW(WmsLogTag::DMS, "success, mainScreenId = %{public}" PRIu64
        ", secondaryScreenMode = %{public}d", lastScreenMode_.first, lastScreenMode_.second);
}

void MultiScreenManager::InternalScreenOnChange(sptr<ScreenSession> internalSession,
    sptr<ScreenSession> externalSession)
{
    if (internalSession == nullptr || externalSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "internal or external screen is null!");
        return;
    }
    ScreenId mainScreenId = lastScreenMode_.first;
    MultiScreenMode secondaryScreenMode = lastScreenMode_.second;
    if (mainScreenId == SCREEN_ID_INVALID) {
        TLOGW(WmsLogTag::DMS, "mode not restored, reset last screen mode");
        ScreenSessionManager::GetInstance().SetLastScreenMode(internalSession, externalSession);
        return;
    }
    ScreenId internalScreenId = ScreenSessionManager::GetInstance().GetInternalScreenId();
    if (mainScreenId == internalScreenId && secondaryScreenMode == MultiScreenMode::SCREEN_MIRROR) {
        DoFirstMirrorChange(internalSession, externalSession, SCREEN_MIRROR);
        TLOGW(WmsLogTag::DMS, "5: external mirror to internal mirror");
    } else if (mainScreenId == internalScreenId && secondaryScreenMode == MultiScreenMode::SCREEN_EXTEND) {
        DoFirstMirrorChange(internalSession, externalSession, SCREEN_EXTEND);
        TLOGW(WmsLogTag::DMS, "7: external mirror to internal extend");
    } else if (mainScreenId != internalScreenId && secondaryScreenMode == MultiScreenMode::SCREEN_EXTEND) {
        DoFirstMainChange(externalSession, internalSession, SCREEN_EXTEND);
        TLOGW(WmsLogTag::DMS, "6: external mirror to external extend");
    } else {
        TLOGE(WmsLogTag::DMS, "no need to change or paramater error!");
    }
    if (secondaryScreenMode == MultiScreenMode::SCREEN_MIRROR) {
        MultiScreenReportDataToRss(SCREEN_MIRROR, MULTI_SCREEN_ENTER_STR);
    } else if (secondaryScreenMode == MultiScreenMode::SCREEN_EXTEND) {
        MultiScreenReportDataToRss(SCREEN_EXTEND, MULTI_SCREEN_ENTER_STR);
    }
}

void MultiScreenManager::InternalScreenOffChange(sptr<ScreenSession> internalSession,
    sptr<ScreenSession> externalSession)
{
    if (internalSession == nullptr || externalSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "internal or external screen is null!");
        return;
    }
    ScreenSessionManager::GetInstance().SetLastScreenMode(internalSession, externalSession);
    ScreenId mainScreenId = lastScreenMode_.first;
    MultiScreenMode secondaryScreenMode = lastScreenMode_.second;
    ScreenId internalScreenId = ScreenSessionManager::GetInstance().GetInternalScreenId();
    if (mainScreenId == internalScreenId && secondaryScreenMode == MultiScreenMode::SCREEN_MIRROR) {
        DoFirstMirrorChange(externalSession, internalSession, SCREEN_MIRROR);
        MultiScreenReportDataToRss(SCREEN_MIRROR, MULTI_SCREEN_EXIT_STR);
        TLOGW(WmsLogTag::DMS, "3: internal mirror to external mirror");
    } else if (mainScreenId == internalScreenId && secondaryScreenMode == MultiScreenMode::SCREEN_EXTEND) {
        DoFirstExtendChange(externalSession, internalSession, SCREEN_MIRROR);
        MultiScreenReportDataToRss(SCREEN_EXTEND, MULTI_SCREEN_EXIT_STR);
        TLOGW(WmsLogTag::DMS, "10: internal extend to external mirror");
    } else if (mainScreenId != internalScreenId && secondaryScreenMode == MultiScreenMode::SCREEN_EXTEND) {
        DoFirstMainChange(externalSession, internalSession, SCREEN_MIRROR);
        MultiScreenReportDataToRss(SCREEN_EXTEND, MULTI_SCREEN_EXIT_STR);
        TLOGW(WmsLogTag::DMS, "14: external extend to external mirror");
    } else {
        MultiScreenReportDataToRss(SCREEN_MIRROR, MULTI_SCREEN_EXIT_STR);
        TLOGE(WmsLogTag::DMS, "no need to change or paramater error!");
    }
}

void MultiScreenManager::ExternalScreenDisconnectChange(sptr<ScreenSession> internalSession,
    sptr<ScreenSession> externalSession)
{
    if (internalSession == nullptr || externalSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "internal or external screen is null!");
        return;
    }
    ScreenCombination internalCombination = internalSession->GetScreenCombination();
    ScreenCombination externalCombination = externalSession->GetScreenCombination();
    if (externalCombination != ScreenCombination::SCREEN_MAIN) {
        DoFirstMainChange(internalSession, externalSession, SCREEN_MIRROR);
        TLOGW(WmsLogTag::DMS, "12: internal extend to internal mirror");
        return;
    }
    if (internalCombination == ScreenCombination::SCREEN_MIRROR) {
        DoFirstMirrorChange(internalSession, externalSession, SCREEN_MIRROR);
        TLOGW(WmsLogTag::DMS, "8: external mirror to internal mirror");
    } else if (internalCombination == ScreenCombination::SCREEN_EXTEND) {
        DoFirstExtendChange(internalSession, externalSession, SCREEN_MIRROR);
        TLOGW(WmsLogTag::DMS, "16: external extend to internal mirror");
    } else {
        TLOGE(WmsLogTag::DMS, "paramater error!");
        return;
    }
}

bool MultiScreenManager::AreScreensTouching(sptr<ScreenSession> mainScreenSession,
    sptr<ScreenSession> secondScreenSession, MultiScreenPositionOptions mainScreenOptions,
    MultiScreenPositionOptions secondScreenOption)
{
    ScreenProperty mainProperty = mainScreenSession->GetScreenProperty();
    uint32_t mainScreenWidth = static_cast<uint32_t>(mainProperty.GetBounds().rect_.GetWidth());
    uint32_t mainScreenHeight = static_cast<uint32_t>(mainProperty.GetBounds().rect_.GetHeight());

    ScreenProperty secondProperty = secondScreenSession->GetScreenProperty();
    uint32_t secondScreenWidth = static_cast<uint32_t>(secondProperty.GetBounds().rect_.GetWidth());
    uint32_t secondScreenHeight = static_cast<uint32_t>(secondProperty.GetBounds().rect_.GetHeight());

    bool horizontalTouchingAB = (mainScreenOptions.startX_ + mainScreenWidth == secondScreenOption.startX_);
    bool horizontalTouchingBA = (secondScreenOption.startX_ + secondScreenWidth == mainScreenOptions.startX_);
    bool horizontalTouching = (horizontalTouchingAB || horizontalTouchingBA);

    bool verticalTouchingAB = (mainScreenOptions.startY_ + mainScreenHeight == secondScreenOption.startY_);
    bool verticalTouchingBA = (secondScreenOption.startY_ + secondScreenHeight == mainScreenOptions.startY_);
    bool verticalTouching = (verticalTouchingAB || verticalTouchingBA);

    if ((horizontalTouchingAB && verticalTouchingAB) || (horizontalTouchingBA && verticalTouchingBA)) {
        return false;
    }
    return horizontalTouching || verticalTouching;
}

void MultiScreenManager::MultiScreenReportDataToRss(std::string multiScreenType, std::string status)
{
#ifdef RES_SCHED_ENABLE
    TLOGI(WmsLogTag::DMS, "type:%{public}s, status:%{public}s", multiScreenType.c_str(), status.c_str());
    uint32_t type = OHOS::ResourceSchedule::ResType::RES_TYPE_DISPLAY_MULTI_SCREEN;
    std::unordered_map<std::string, std::string> payload = {
        { "type", multiScreenType },
        { "status", status },
    };
    int32_t value = status == MULTI_SCREEN_ENTER_STR ? MULTI_SCREEN_ENTER : MULTI_SCREEN_EXIT;
    OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, value, payload);
#endif
}
} // namespace OHOS::Rosen
