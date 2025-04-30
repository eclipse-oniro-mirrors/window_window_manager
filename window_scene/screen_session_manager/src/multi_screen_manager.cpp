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
#include "screen_power_utils.h"
#include "screen_scene_config.h"
#ifdef RES_SCHED_ENABLE
#include "res_sched_client.h"
#include "res_type.h"
#endif

namespace OHOS::Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(MultiScreenManager)
namespace {
const std::string SCREEN_EXTEND = "extend";
const std::string SCREEN_MIRROR = "mirror";
const std::string MULTI_SCREEN_EXIT_STR = "exit";
const std::string MULTI_SCREEN_ENTER_STR = "enter";
constexpr int32_t MULTI_SCREEN_EXIT = 0;
constexpr int32_t MULTI_SCREEN_ENTER = 1;
}
MultiScreenManager::MultiScreenManager()
{
    TLOGI(WmsLogTag::DMS_SSM, "init");
    lastScreenMode_ = std::make_pair(SCREEN_ID_INVALID, MultiScreenMode::SCREEN_MIRROR);
}

MultiScreenManager::~MultiScreenManager()
{
    TLOGI(WmsLogTag::DMS_SSM, "destructor");
}

void MultiScreenManager::FilterPhysicalAndVirtualScreen(const std::vector<ScreenId>& allScreenIds,
    std::vector<ScreenId>& physicalScreenIds, std::vector<ScreenId>& virtualScreenIds)
{
    TLOGW(WmsLogTag::DMS_SSM, "enter allScreen size: %{public}u",
        static_cast<uint32_t>(allScreenIds.size()));
    sptr<ScreenSession> defaultSession = ScreenSessionManager::GetInstance().GetDefaultScreenSession();
    if (defaultSession == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "fail to get defaultSession");
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
            TLOGW(WmsLogTag::DMS_SSM, "mirror screen type error");
        }
    }
    TLOGW(WmsLogTag::DMS_SSM, "end");
}

DMError MultiScreenManager::VirtualScreenMirrorSwitch(const ScreenId mainScreenId,
    const std::vector<ScreenId>& screenIds, DMRect mainScreenRegion, ScreenId& screenGroupId)
{
    TLOGW(WmsLogTag::DMS_SSM, "enter size: %{public}u",
        static_cast<uint32_t>(screenIds.size()));
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:VirtualScreenMirrorSwitch start");
    auto mainScreen = ScreenSessionManager::GetInstance().GetScreenSession(mainScreenId);
    if (mainScreen == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "screen session null fail mainScreenId: %{public}" PRIu64, mainScreenId);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    DMError ret = ScreenSessionManager::GetInstance().SetMirror(mainScreenId, screenIds, mainScreenRegion);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS_SSM, "error: %{public}d", ret);
        return ret;
    }
    if (ScreenSessionManager::GetInstance().GetAbstractScreenGroup(mainScreen->groupSmsId_) == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "get screen group failed main screenId: %{public}" PRIu64, mainScreenId);
        return DMError::DM_ERROR_NULLPTR;
    }
    screenGroupId = mainScreen->groupSmsId_;
    TLOGW(WmsLogTag::DMS_SSM, "end");
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:VirtualScreenMirrorSwitch end");
    return ret;
}

DMError MultiScreenManager::PhysicalScreenMirrorSwitch(const std::vector<ScreenId>& screenIds, DMRect mirrorRegion)
{
    sptr<ScreenSession> defaultSession = ScreenSessionManager::GetInstance().GetDefaultScreenSession();
    if (defaultSession == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "fail to get defaultSession");
        return DMError::DM_ERROR_NULLPTR;
    }
    TLOGW(WmsLogTag::DMS_SSM, "enter physical screen switch to mirror screen size: %{public}u",
        static_cast<uint32_t>(screenIds.size()));
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:PhysicalScreenMirrorSwitch start");
    NodeId nodeId = defaultSession->GetDisplayNode() == nullptr ? 0 : defaultSession->GetDisplayNode()->GetId();
    for (ScreenId physicalScreenId : screenIds) {
        auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(physicalScreenId);
        if (screenSession == nullptr) {
            continue;
        }
        TLOGW(WmsLogTag::DMS_SSM, "switch to mirror physical ScreenId: %{public}" PRIu64, physicalScreenId);
        std::shared_ptr<RSDisplayNode> displayNode = screenSession->GetDisplayNode();
        if (screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
            if (mirrorRegion != screenSession->GetMirrorScreenRegion().second) {
                screenSession->SetMirrorScreenRegion(defaultSession->GetRSScreenId(), mirrorRegion);
                screenSession->SetIsPhysicalMirrorSwitch(true);
                screenSession->EnableMirrorScreenRegion();
            }
            TLOGW(WmsLogTag::DMS_SSM, "already mirror and get a same region.");
            return DMError::DM_OK;
        }
        if (displayNode != nullptr) {
            displayNode->RemoveFromTree();
        }
        screenSession->ReleaseDisplayNode();
        screenSession->SetMirrorScreenRegion(defaultSession->GetRSScreenId(), mirrorRegion);
        screenSession->SetIsPhysicalMirrorSwitch(true);
        RSDisplayNodeConfig config = { screenSession->screenId_, true, nodeId, true };
        screenSession->CreateDisplayNode(config);
        screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    }
    TLOGW(WmsLogTag::DMS_SSM, "physical screen switch to mirror end");
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:PhysicalScreenMirrorSwitch end");
    return DMError::DM_OK;
}

DMError MultiScreenManager::PhysicalScreenUniqueSwitch(const std::vector<ScreenId>& screenIds)
{
    TLOGW(WmsLogTag::DMS_SSM, "enter screen size: %{public}u",
        static_cast<uint32_t>(screenIds.size()));
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:PhysicalScreenUniqueSwitch start");
    for (ScreenId physicalScreenId : screenIds) {
        auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(physicalScreenId);
        if (screenSession == nullptr) {
            continue;
        }
        TLOGW(WmsLogTag::DMS_SSM, "switch to unique physical ScreenId: %{public}" PRIu64, physicalScreenId);
        std::shared_ptr<RSDisplayNode> displayNode = screenSession->GetDisplayNode();
        if (displayNode != nullptr) {
            displayNode->RemoveFromTree();
        }
        screenSession->ReleaseDisplayNode();
        screenSession->SetVirtualPixelRatio(screenSession->GetScreenProperty().GetDefaultDensity());
        RSDisplayNodeConfig config = { screenSession->screenId_ };
        screenSession->CreateDisplayNode(config);
        ScreenSessionManager::GetInstance().OnVirtualScreenChange(physicalScreenId, ScreenEvent::CONNECTED);
    }
    TLOGW(WmsLogTag::DMS_SSM, "end");
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:PhysicalScreenUniqueSwitch end");
    return DMError::DM_OK;
}

DMError MultiScreenManager::VirtualScreenUniqueSwitch(sptr<ScreenSession> screenSession,
    const std::vector<ScreenId>& screenIds)
{
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "screenSession is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:VirtualScreenUniqueSwitch start");
    TLOGW(WmsLogTag::DMS_SSM, "start size: %{public}u",
        static_cast<uint32_t>(screenIds.size()));
    auto group = ScreenSessionManager::GetInstance().GetAbstractScreenGroup(screenSession->groupSmsId_);
    if (group == nullptr) {
        group = ScreenSessionManager::GetInstance().AddToGroupLocked(screenSession, true);
        if (group == nullptr) {
            TLOGE(WmsLogTag::DMS_SSM, "group is nullptr");
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
            ScreenSessionManager::GetInstance().NotifyScreenChanged(uniqueScreen->ConvertToScreenInfo(),
                ScreenChangeEvent::SCREEN_SWITCH_CHANGE);
        }
        // virtual screen create callback to notify scb
        ScreenSessionManager::GetInstance().OnVirtualScreenChange(uniqueScreenId, ScreenEvent::CONNECTED);
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:VirtualScreenUniqueSwitch end");
    TLOGW(WmsLogTag::DMS_SSM, "to unique and notify scb end");
    return DMError::DM_OK;
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

DMError MultiScreenManager::UniqueSwitch(const std::vector<ScreenId>& screenIds,
    std::vector<DisplayId>& displayIds)
{
    DMError switchStatus = DMError::DM_OK;
    std::vector<ScreenId> virtualScreenIds;
    std::vector<ScreenId> physicalScreenIds;
    if (screenIds.empty()) {
        TLOGW(WmsLogTag::DMS_SSM, "mirror to screen size empty");
        return switchStatus;
    }
    TLOGW(WmsLogTag::DMS_SSM, "enter mirror to screen size: %{public}u",
        static_cast<uint32_t>(screenIds.size()));
    FilterPhysicalAndVirtualScreen(screenIds, physicalScreenIds, virtualScreenIds);

    if (!virtualScreenIds.empty()) {
        switchStatus = ScreenSessionManager::GetInstance().VirtualScreenUniqueSwitch(virtualScreenIds);
        TLOGW(WmsLogTag::DMS_SSM, "virtual screen switch to unique result: %{public}d", switchStatus);
        AddUniqueScreenDisplayId(displayIds, virtualScreenIds, switchStatus);
    }
    if (!physicalScreenIds.empty()) {
        switchStatus = PhysicalScreenUniqueSwitch(physicalScreenIds);
        if (switchStatus == DMError::DM_OK) {
            for (auto screenId : physicalScreenIds) {
                ScreenSessionManager::GetInstance().UnregisterSettingWireCastObserver(screenId);
            }
        }
        if (!ScreenSessionManager::GetInstance().HasCastEngineOrPhyMirror(physicalScreenIds) &&
            switchStatus == DMError::DM_OK && ScreenSceneConfig::GetExternalScreenDefaultMode() != "none") {
            ScreenPowerUtils::DisablePowerForceTimingOut();
        }
        TLOGW(WmsLogTag::DMS_SSM, "physical screen switch to unique result: %{public}d", switchStatus);
        AddUniqueScreenDisplayId(displayIds, physicalScreenIds, switchStatus);
    }
    if (displayIds.empty()) {
        switchStatus = DMError::DM_ERROR_INVALID_PARAM;
    }
    TLOGW(WmsLogTag::DMS_SSM, "mirror switch to unique end");
    return switchStatus;
}

DMError MultiScreenManager::MirrorSwitch(const ScreenId mainScreenId, const std::vector<ScreenId>& screenIds,
    DMRect mainScreenRegion, ScreenId& screenGroupId)
{
    DMError switchStatus = DMError::DM_OK;
    std::vector<ScreenId> virtualScreenIds;
    std::vector<ScreenId> physicalScreenIds;
    if (screenIds.empty()) {
        TLOGW(WmsLogTag::DMS_SSM, "screen size empty");
        return switchStatus;
    }
    TLOGW(WmsLogTag::DMS_SSM, "enter screen size: %{public}u", static_cast<uint32_t>(screenIds.size()));
    FilterPhysicalAndVirtualScreen(screenIds, physicalScreenIds, virtualScreenIds);

    if (!virtualScreenIds.empty()) {
        switchStatus = VirtualScreenMirrorSwitch(mainScreenId, virtualScreenIds, mainScreenRegion, screenGroupId);
        TLOGW(WmsLogTag::DMS_SSM, "virtual screen switch to mirror result: %{public}d", switchStatus);
    }
    if (!physicalScreenIds.empty()) {
        screenGroupId = 1;
        switchStatus = PhysicalScreenMirrorSwitch(physicalScreenIds, mainScreenRegion);
        if (switchStatus == DMError::DM_OK) {
            for (auto screenId : physicalScreenIds) {
                auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
                ScreenSessionManager::GetInstance().RegisterSettingWireCastObserver(screenSession);
            }
            // switchStatus is DM_OK and device is not PC
            if (ScreenSceneConfig::GetExternalScreenDefaultMode() != "none") {
                ScreenPowerUtils::EnablePowerForceTimingOut();
                ScreenSessionManager::GetInstance().DisablePowerOffRenderControl(0);
            }
        }
        TLOGW(WmsLogTag::DMS_SSM, "physical screen switch to mirror result: %{public}d", switchStatus);
    }
    TLOGW(WmsLogTag::DMS_SSM, "end switchStatus: %{public}d", switchStatus);
    return switchStatus;
}

void MultiScreenManager::MultiScreenModeChange(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession,
    const std::string& operateType)
{
    TLOGW(WmsLogTag::DMS_SSM, "enter operateType=%{public}s", operateType.c_str());
    if (firstSession == nullptr || secondarySession == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "params null.");
        return;
    }
    ScreenCombination firstCombination = firstSession->GetScreenCombination();
    if (firstCombination == ScreenCombination::SCREEN_MAIN) {
        /* second screen change to mirror or extend */
        TLOGW(WmsLogTag::DMS_SSM, "first is already main");
        DoFirstMainChange(firstSession, secondarySession, operateType);
    } else if (firstCombination == ScreenCombination::SCREEN_MIRROR) {
        /* first screen change from mirror to main */
        TLOGW(WmsLogTag::DMS_SSM, "first screen change from mirror to main");
        DoFirstMirrorChange(firstSession, secondarySession, operateType);
    } else if (firstCombination == ScreenCombination::SCREEN_EXTEND) {
        /* first screen change from extend to main */
        TLOGW(WmsLogTag::DMS_SSM, "first screen change from extend to main");
        DoFirstExtendChange(firstSession, secondarySession, operateType);
    } else {
        TLOGE(WmsLogTag::DMS_SSM, "first screen mode error");
    }
}

void MultiScreenManager::DoFirstMainChangeExtend(sptr<IScreenSessionManagerClient> scbClient,
    sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession)
{
    TLOGW(WmsLogTag::DMS_SSM, "exec switch extend");
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    RSDisplayNodeConfig config = { secondarySession->screenId_, false, INVALID_NODEID};
    secondarySession->ReuseDisplayNode(config);
    secondarySession->SetIsExtend(true);
    firstSession->SetIsExtend(false);
    SessionOption secondaryOption = ScreenSessionManager::GetInstance().GetSessionOption(secondarySession);
    scbClient->OnScreenConnectionChanged(secondaryOption, ScreenEvent::CONNECTED);
    ScreenSessionManager::GetInstance().NotifyScreenChanged(
        secondarySession->ConvertToScreenInfo(), ScreenChangeEvent::SCREEN_SOURCE_MODE_CHANGE);
    TLOGW(WmsLogTag::DMS_SSM, "exec switch mirror to extend 4/6 end");
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
    TLOGW(WmsLogTag::DMS_SSM, "exec switch mirror 12/14 end");
}

void MultiScreenManager::DoFirstMainChange(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession,
    const std::string& operateType)
{
    TLOGW(WmsLogTag::DMS_SSM, "enter");
    sptr<IScreenSessionManagerClient> scbClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (scbClient == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "scbClient null");
        return;
    }
    firstSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    ScreenCombination secondaryCombination = secondarySession->GetScreenCombination();
    TLOGW(WmsLogTag::DMS_SSM, "current secondary screen mode:%{public}d", secondaryCombination);
    if (operateType == SCREEN_EXTEND) {
        TLOGW(WmsLogTag::DMS_SSM, "exec switch extend");
        if (secondaryCombination == ScreenCombination::SCREEN_MIRROR) {
            // mirror to extend 4,6
            DoFirstMainChangeExtend(scbClient, firstSession, secondarySession);
        } else {
            TLOGW(WmsLogTag::DMS_SSM, "already extend no need to change");
        }
    } else if (operateType == SCREEN_MIRROR) {
        TLOGW(WmsLogTag::DMS_SSM, "exec switch mirror");
        if (secondaryCombination == ScreenCombination::SCREEN_EXTEND) {
            // mirror to extend 12,14
            DoFirstMainChangeMirror(scbClient, firstSession, secondarySession);
        } else {
            TLOGE(WmsLogTag::DMS_SSM, "already mirror no need to change");
        }
    } else {
        TLOGE(WmsLogTag::DMS_SSM, "param error!");
    }
}

void MultiScreenManager::DoFirstMirrorChangeExtend(sptr<IScreenSessionManagerClient> scbClient,
    sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession)
{
    /* change secondarySession to extend */
    TLOGW(WmsLogTag::DMS_SSM, "exec switch extend");
    secondarySession->SetIsExtend(true);
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);

    /* change firstSession from mirror to main */
    std::shared_ptr<RSDisplayNode> displayNode = firstSession->GetDisplayNode();
    if (displayNode != nullptr) {
        displayNode->RemoveFromTree();
    }
    firstSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    firstSession->ReleaseDisplayNode();
    RSDisplayNodeConfig config = { firstSession->screenId_ };
    firstSession->SetIsExtend(false);
    firstSession->CreateDisplayNode(config);
    ScreenSessionManager::GetInstance().SetDefaultScreenId(firstSession->GetScreenId());
    SessionOption firstOption = ScreenSessionManager::GetInstance().GetSessionOption(firstSession);
    scbClient->OnScreenConnectionChanged(firstOption, ScreenEvent::CONNECTED);
    /* move secondarySession windows to firstSession */
    TLOGW(WmsLogTag::DMS_SSM, "mainScreenId:%{public}" PRIu64", extendScreenId=%{public}" PRIu64,
        firstSession->GetScreenId(), secondarySession->GetScreenId());
    secondarySession->ScreenExtendChange(firstSession->GetScreenId(), secondarySession->GetScreenId());
    TLOGW(WmsLogTag::DMS_SSM, "exec switch extend 1/7 end");
}

void MultiScreenManager::DoFirstMirrorChangeMirror(sptr<IScreenSessionManagerClient> scbClient,
    sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession)
{
    /* change firstSession from to mirror */
    TLOGW(WmsLogTag::DMS_SSM, "exec switch mirror");
    std::shared_ptr<RSDisplayNode> displayNode = firstSession->GetDisplayNode();
    if (displayNode != nullptr) {
        displayNode->RemoveFromTree();
    }
    firstSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    firstSession->ReleaseDisplayNode();
    RSDisplayNodeConfig config = { firstSession->screenId_ };
    firstSession->SetIsExtend(false);
    firstSession->CreateDisplayNode(config);
    ScreenSessionManager::GetInstance().SetDefaultScreenId(firstSession->GetScreenId());
    SessionOption firstOption = ScreenSessionManager::GetInstance().GetSessionOption(firstSession);
    scbClient->OnScreenConnectionChanged(firstOption, ScreenEvent::CONNECTED);
    /* move secondarySession windows to firstSession */
    /* change secondarySession to mirror */
    /* create mirror */
    SessionOption secondaryOption = ScreenSessionManager::GetInstance().GetSessionOption(secondarySession);
    scbClient->OnScreenConnectionChanged(secondaryOption, ScreenEvent::DISCONNECTED);
    /* create inner screen's mirror */
    NodeId nodeId = firstSession->GetDisplayNode() == nullptr ? 0 : firstSession->GetDisplayNode()->GetId();
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    secondarySession->SetIsExtend(true);
    config = {secondarySession->screenId_, true, nodeId };
    secondarySession->ReuseDisplayNode(config);
    TLOGW(WmsLogTag::DMS_SSM, "exec switch mirror 2/3/5/8 end");
}

void MultiScreenManager::DoFirstMirrorChange(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession,
    const std::string& operateType)
{
    TLOGW(WmsLogTag::DMS_SSM, "enter");
    sptr<IScreenSessionManagerClient> scbClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (scbClient == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "scbClient null");
        return;
    }
    if (operateType == SCREEN_EXTEND) {
        // 1, 7
        DoFirstMirrorChangeExtend(scbClient, firstSession, secondarySession);
    } else if (operateType == SCREEN_MIRROR) {
        // 2 3 5 8
        DoFirstMirrorChangeMirror(scbClient, firstSession, secondarySession);
    } else {
        TLOGE(WmsLogTag::DMS_SSM, "param error!");
    }
}

void MultiScreenManager::DoFirstExtendChangeExtend(sptr<ScreenSession> firstSession,
    sptr<ScreenSession> secondarySession)
{
    /* set firstSession main */
    TLOGW(WmsLogTag::DMS_SSM, "exec switch extend");
    firstSession->SetIsExtend(false);
    firstSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    ScreenSessionManager::GetInstance().SetDefaultScreenId(firstSession->GetScreenId());
    /* set secondarySession extend */
    secondarySession->SetIsExtend(true);
    secondarySession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    TLOGW(WmsLogTag::DMS_SSM, "mainScreenId:%{public}" PRIu64", extendScreenId=%{public}" PRIu64,
        firstSession->GetScreenId(), secondarySession->GetScreenId());
    firstSession->ScreenExtendChange(firstSession->GetScreenId(), secondarySession->GetScreenId());
    TLOGW(WmsLogTag::DMS_SSM, "exec switch extend 9/11/13/15 end");
    /* change main and extend screens's windows */
}

void MultiScreenManager::DoFirstExtendChangeMirror(sptr<ScreenSession> firstSession,
    sptr<ScreenSession> secondarySession)
{
    sptr<IScreenSessionManagerClient> scbClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (scbClient == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "scbClient null");
        return;
    }
    /* set firstSession main screen */
    TLOGW(WmsLogTag::DMS_SSM, "exec switch mirror");
    firstSession->SetIsExtend(false);
    firstSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    ScreenSessionManager::GetInstance().SetDefaultScreenId(firstSession->GetScreenId());
    TLOGW(WmsLogTag::DMS_SSM, "mainScreenId:%{public}" PRIu64", extendScreenId=%{public}" PRIu64,
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
    TLOGW(WmsLogTag::DMS_SSM, "exec switch mirror 10/16 end");
}

void MultiScreenManager::DoFirstExtendChange(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession,
    const std::string& operateType)
{
    TLOGW(WmsLogTag::DMS_SSM, "enter");
    if (operateType == SCREEN_EXTEND) {
        // 9 11 13 15
        DoFirstExtendChangeExtend(firstSession, secondarySession);
    } else if (operateType == SCREEN_MIRROR) {
        // 10 16
        DoFirstExtendChangeMirror(firstSession, secondarySession);
    } else {
        TLOGE(WmsLogTag::DMS_SSM, "param error!");
    }
}

void MultiScreenManager::SetLastScreenMode(ScreenId mainScreenId, MultiScreenMode secondaryScreenMode)
{
    lastScreenMode_.first = mainScreenId;
    lastScreenMode_.second = secondaryScreenMode;
    TLOGW(WmsLogTag::DMS_SSM, "success, mainScreenId = %{public}" PRIu64
        ", secondaryScreenMode = %{public}d", lastScreenMode_.first, lastScreenMode_.second);
}

void MultiScreenManager::InternalScreenOnChange(sptr<ScreenSession> internalSession,
    sptr<ScreenSession> externalSession)
{
    if (internalSession == nullptr || externalSession == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "internal or external screen is null!");
        return;
    }
    ScreenId mainScreenId = lastScreenMode_.first;
    MultiScreenMode secondaryScreenMode = lastScreenMode_.second;
    if (mainScreenId == SCREEN_ID_INVALID) {
        TLOGW(WmsLogTag::DMS_SSM, "mode not restored, reset last screen mode");
        ScreenSessionManager::GetInstance().SetLastScreenMode(internalSession, externalSession);
        return;
    }
    ScreenId internalScreenId = ScreenSessionManager::GetInstance().GetInternalScreenId();
    if (mainScreenId == internalScreenId && secondaryScreenMode == MultiScreenMode::SCREEN_MIRROR) {
        DoFirstMirrorChange(internalSession, externalSession, SCREEN_MIRROR);
        TLOGW(WmsLogTag::DMS_SSM, "5: external mirror to internal mirror");
    } else if (mainScreenId == internalScreenId && secondaryScreenMode == MultiScreenMode::SCREEN_EXTEND) {
        DoFirstMirrorChange(internalSession, externalSession, SCREEN_EXTEND);
        TLOGW(WmsLogTag::DMS_SSM, "7: external mirror to internal extend");
    } else if (mainScreenId != internalScreenId && secondaryScreenMode == MultiScreenMode::SCREEN_EXTEND) {
        DoFirstMainChange(externalSession, internalSession, SCREEN_EXTEND);
        TLOGW(WmsLogTag::DMS_SSM, "6: external mirror to external extend");
    } else {
        TLOGE(WmsLogTag::DMS_SSM, "no need to change or paramater error!");
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
        TLOGE(WmsLogTag::DMS_SSM, "internal or external screen is null!");
        return;
    }
    ScreenSessionManager::GetInstance().SetLastScreenMode(internalSession, externalSession);
    ScreenId mainScreenId = lastScreenMode_.first;
    MultiScreenMode secondaryScreenMode = lastScreenMode_.second;
    ScreenId internalScreenId = ScreenSessionManager::GetInstance().GetInternalScreenId();
    if (mainScreenId == internalScreenId && secondaryScreenMode == MultiScreenMode::SCREEN_MIRROR) {
        DoFirstMirrorChange(externalSession, internalSession, SCREEN_MIRROR);
        MultiScreenReportDataToRss(SCREEN_MIRROR, MULTI_SCREEN_EXIT_STR);
        TLOGW(WmsLogTag::DMS_SSM, "3: internal mirror to external mirror");
    } else if (mainScreenId == internalScreenId && secondaryScreenMode == MultiScreenMode::SCREEN_EXTEND) {
        DoFirstExtendChange(externalSession, internalSession, SCREEN_MIRROR);
        MultiScreenReportDataToRss(SCREEN_EXTEND, MULTI_SCREEN_EXIT_STR);
        TLOGW(WmsLogTag::DMS_SSM, "10: internal extend to external mirror");
    } else if (mainScreenId != internalScreenId && secondaryScreenMode == MultiScreenMode::SCREEN_EXTEND) {
        DoFirstMainChange(externalSession, internalSession, SCREEN_MIRROR);
        MultiScreenReportDataToRss(SCREEN_EXTEND, MULTI_SCREEN_EXIT_STR);
        TLOGW(WmsLogTag::DMS_SSM, "14: external extend to external mirror");
    } else {
        MultiScreenReportDataToRss(SCREEN_MIRROR, MULTI_SCREEN_EXIT_STR);
        TLOGE(WmsLogTag::DMS_SSM, "no need to change or paramater error!");
    }
}

void MultiScreenManager::ExternalScreenDisconnectChange(sptr<ScreenSession> internalSession,
    sptr<ScreenSession> externalSession)
{
    if (internalSession == nullptr || externalSession == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "internal or external screen is null!");
        return;
    }
    ScreenCombination internalCombination = internalSession->GetScreenCombination();
    ScreenCombination externalCombination = externalSession->GetScreenCombination();
    if (externalCombination != ScreenCombination::SCREEN_MAIN) {
        DoFirstMainChange(internalSession, externalSession, SCREEN_MIRROR);
        TLOGW(WmsLogTag::DMS_SSM, "12: internal extend to internal mirror");
        return;
    }
    if (internalCombination == ScreenCombination::SCREEN_MIRROR) {
        DoFirstMirrorChange(internalSession, externalSession, SCREEN_MIRROR);
        TLOGW(WmsLogTag::DMS_SSM, "8: external mirror to internal mirror");
    } else if (internalCombination == ScreenCombination::SCREEN_EXTEND) {
        DoFirstExtendChange(internalSession, externalSession, SCREEN_MIRROR);
        TLOGW(WmsLogTag::DMS_SSM, "16: external extend to internal mirror");
    } else {
        TLOGE(WmsLogTag::DMS_SSM, "paramater error!");
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
    TLOGI(WmsLogTag::DMS_SSM, "type:%{public}s, status:%{public}s", multiScreenType.c_str(), status.c_str());
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
