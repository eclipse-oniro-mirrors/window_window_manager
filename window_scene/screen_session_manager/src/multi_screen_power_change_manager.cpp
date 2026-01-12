/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "multi_screen_power_change_manager.h"

#include <hitrace_meter.h>
#include <transaction/rs_transaction.h>

#include "rs_adapter.h"
#include "window_manager_hilog.h"
#include <power_mgr_client.h>

namespace OHOS::Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(MultiScreenPowerChangeManager)
namespace {
const ScreenId SCREEN_ID_FULL = 0;
} // namespace

void MultiScreenPowerChangeManager::InitMultiScreenPowerChangeMap()
{
    if (handleScreenPowerChangeMap_.size() != 0) {
        TLOGW(WmsLogTag::DMS, "multi screen power change map has init!");
        return;
    }
    /* screen switch open */
    handleScreenPowerChangeMap_[MultiScreenPowerSwitchType::SCREEN_SWITCH_ON] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleScreenOnChange(innerScreen, externalScreen);
        };
    /* screen switch close */
    handleScreenPowerChangeMap_[MultiScreenPowerSwitchType::SCREEN_SWITCH_OFF] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
        return HandleScreenOffChange(innerScreen, externalScreen);
    };
    /* setting switch external screen */
    handleScreenPowerChangeMap_[MultiScreenPowerSwitchType::SCREEN_SWITCH_EXTERNAL] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
        return HandleScreenOnlyExternalModeChange(innerScreen, externalScreen);
    };
}

DMError MultiScreenPowerChangeManager::OnMultiScreenPowerChangeRequest(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen, MultiScreenPowerSwitchType switchStatus)
{
    DMError handleRet = DMError::DM_OK;
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "parameters nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    auto handleCall = handleScreenPowerChangeMap_.find(switchStatus);
    if (handleCall != handleScreenPowerChangeMap_.end() && handleCall->second != nullptr) {
        auto powerHandleFunction = handleCall->second;
        handleRet = powerHandleFunction(innerScreen, externalScreen);
    } else {
        TLOGE(WmsLogTag::DMS, "failed to find function handler!");
        handleRet = DMError::DM_ERROR_INVALID_CALLING;
    }
    return handleRet;
}

DMError MultiScreenPowerChangeManager::HandleScreenOnChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "parameters nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    std::ostringstream oss;
    oss << "inner screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", isExtend: " << (innerScreen->GetIsExtend() ? "true" : "false")
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId()
        << ", isExtend: " << (externalScreen->GetIsExtend() ? "true" : "false");
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    return OnRecoveryScreenModeByPowerChange(innerScreen, externalScreen);
}

DMError MultiScreenPowerChangeManager::HandleScreenOffChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "parameters nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    innerCombination_ = innerScreen->GetScreenCombination();
    externalCombination_ = externalScreen->GetScreenCombination();

    std::ostringstream oss;
    oss << "inner screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", combination: " << static_cast<int32_t>(innerCombination_)
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId()
        << ", combination: " << static_cast<int32_t>(externalCombination_);
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    return HandleScreenOnlyExternalModeChange(innerScreen, externalScreen);
}

void MultiScreenPowerChangeManager::InitRecoveryMultiScreenModeChangeMap()
{
    if (handleRecoveryScreenModeChangeMap_.size() != 0) {
        TLOGW(WmsLogTag::DMS, "recovery multi screen mode change map has init!");
        return;
    }
    /* recovery inner main and external extend change from external main. */
    handleRecoveryScreenModeChangeMap_[{ ScreenCombination::SCREEN_MAIN, ScreenCombination::SCREEN_EXTEND}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleRecoveryInnerMainExternalExtendChange(innerScreen, externalScreen);
        };
    /* recovery inner main and external mirror change from external main. */
    handleRecoveryScreenModeChangeMap_[{ ScreenCombination::SCREEN_MIRROR, ScreenCombination::SCREEN_MAIN }] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleRecoveryInnerMirrorExternalMainChange(innerScreen, externalScreen);
        };
    /* recovery inner extend and external main change from external main. */
    handleRecoveryScreenModeChangeMap_[{ ScreenCombination::SCREEN_EXTEND, ScreenCombination::SCREEN_MAIN}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleRecoveryInnerExtendExternalMainChange(innerScreen, externalScreen);
        };
    /* recovery inner mirror and external main change from external main. */
    handleRecoveryScreenModeChangeMap_[{ ScreenCombination::SCREEN_MAIN, ScreenCombination::SCREEN_MIRROR}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleRecoveryInnerMainExternalMirrorChange(innerScreen, externalScreen);
        };
}

void MultiScreenPowerChangeManager::InitMultiScreenModeOffChangeMap()
{
    if (handleScreenOffModeChangeMap_.size() != 0) {
        TLOGW(WmsLogTag::DMS, "multi screen mode change map has init!");
        return;
    }
    /* form inner main and external extend change to external main. */
    handleScreenOffModeChangeMap_[{ ScreenCombination::SCREEN_MAIN, ScreenCombination::SCREEN_EXTEND}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerMainExternalExtendChange(innerScreen, externalScreen);
        };
    /* form inner main and external mirror change to external main. */
    handleScreenOffModeChangeMap_[{ ScreenCombination::SCREEN_MAIN, ScreenCombination::SCREEN_MIRROR}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerMainExternalMirrorChange(innerScreen, externalScreen);
        };
    /* form inner extend and external main change to external main. */
    handleScreenOffModeChangeMap_[{ ScreenCombination::SCREEN_EXTEND, ScreenCombination::SCREEN_MAIN}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerExtendExternalMainChange(innerScreen, externalScreen);
        };
    /* form inner mirror and external main change to external main. */
    handleScreenOffModeChangeMap_[{ ScreenCombination::SCREEN_MIRROR, ScreenCombination::SCREEN_MAIN }] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerMirrorExternalMainChange(innerScreen, externalScreen);
        };
}

DMError MultiScreenPowerChangeManager::HandleScreenOnlyExternalModeChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    DMError handleRet = DMError::DM_OK;
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "parameters nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    ScreenCombination innerCombination = innerScreen->GetScreenCombination();
    ScreenCombination externalCombination = externalScreen->GetScreenCombination();
    auto modeHandleCall = handleScreenOffModeChangeMap_.find({innerCombination, externalCombination});
    if (modeHandleCall != handleScreenOffModeChangeMap_.end()) {
        auto modeHandleFunction = modeHandleCall->second;
        handleRet = modeHandleFunction(innerScreen, externalScreen);
        MultiScreenChangeUtils::SetExternalScreenOffScreenRendering(innerScreen, externalScreen);
    } else {
        TLOGE(WmsLogTag::DMS, "failed to find function handler!");
        handleRet = DMError::DM_ERROR_INVALID_CALLING;
    }
    return handleRet;
}

void MultiScreenPowerChangeManager::ScreenDisplayNodeRemove(sptr<ScreenSession>& screenScreen)
{
    {
        TLOGW(WmsLogTag::DMS, "removeNode screenId=%{public}" PRIu64, screenScreen->GetScreenId());
        std::shared_ptr<RSDisplayNode> displayNode = screenScreen->GetDisplayNode();
        if (displayNode != nullptr) {
            ScreenSessionManager::GetInstance().SetScreenOffset(screenScreen->GetScreenId(), 0, 0);
            displayNode->RemoveFromTree();
            screenScreen->ReleaseDisplayNode();
        }
        displayNode = nullptr;
    }
    RSTransactionAdapter::FlushImplicitTransaction(screenScreen->GetRSUIContext());
}

void MultiScreenPowerChangeManager::ScreenToExtendChange(sptr<IScreenSessionManagerClient> ssmClient,
    sptr<ScreenSession> screenSession)
{
    MultiScreenChangeUtils::CreateExtendSession(screenSession);

    MultiScreenChangeUtils::ScreenConnectionChange(ssmClient, screenSession, ScreenEvent::CONNECTED);
    TLOGW(WmsLogTag::DMS, "screen to extend change end.");
}

void MultiScreenPowerChangeManager::NotifyClientCreateSessionOnly(sptr<IScreenSessionManagerClient> ssmClient,
    sptr<ScreenSession> screenSession)
{
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    ssmClient->OnCreateScreenSessionOnly(screenSession->GetScreenId(), screenSession->GetRSScreenId(),
        screenSession->GetName(), true);
    TLOGW(WmsLogTag::DMS, "notify client create session end.");
}

void MultiScreenPowerChangeManager::CreateExternalScreenDisplayNodeOnly(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen, ScreenCombination combination)
{
    TLOGW(WmsLogTag::DMS, "create extend screen rsId=%{public}" PRIu64, innerScreen->rsId_);
    RSDisplayNodeConfig config = { innerScreen->rsId_, false, INVALID_NODEID};
    if (combination == ScreenCombination::SCREEN_MIRROR) {
        NodeId nodeId = externalScreen->GetDisplayNode() == nullptr ? 0 : externalScreen->GetDisplayNode()->GetId();
        TLOGW(WmsLogTag::DMS, "config screen mirror displayNodeId=%{public}" PRIu64, nodeId);
        config = { innerScreen->rsId_, true, nodeId };
    }
    innerScreen->ReuseDisplayNode(config);
    TLOGW(WmsLogTag::DMS, "create screen displayNode end.");
}

/* inner main and external extend to external main */
DMError MultiScreenPowerChangeManager::HandleInnerMainExternalExtendChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "%s", __func__);
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "client null");
        return DMError::DM_ERROR_NULLPTR;
    }
    TLOGW(WmsLogTag::DMS, "inner main and external extend to external main start.");

    /* step1: change display Node */
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker("InnerMainExternalExtendChange");
    ScreenId innerScreenId = innerScreen->GetScreenId();
    ScreenId externalScreenId = externalScreen->GetScreenId();
    bool changeRet = ssmClient->OnExtendDisplayNodeChange(innerScreenId, externalScreenId);
    if (!changeRet) {
        TLOGE(WmsLogTag::DMS, "change displayNode failed.");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    /* step2: change combination */
    MultiScreenChangeUtils::ScreenCombinationChange(innerScreen, externalScreen, ScreenCombination::SCREEN_EXTEND);

    /* step3: change physical screen info between inner and external screen */
    MultiScreenChangeUtils::ScreenPhysicalInfoChange(innerScreen, externalScreen);

    /* step4: change position */
    MultiScreenChangeUtils::ScreenMainPositionChange(externalScreen, innerScreen);

    /* step5: notify external screen property change */
    MultiScreenChangeUtils::ScreenPropertyChangeNotify(innerScreen, externalScreen);

    /* step6: dpi change */
    MultiScreenChangeUtils::ScreenDensityChangeNotify(innerScreen, externalScreen);

    /* step7: set inner screen unavailable */
    MultiScreenChangeUtils::SetScreenAvailableStatus(externalScreen, false);

    /* step8: disconnect inner screen */
    MultiScreenChangeUtils::ScreenConnectionChange(ssmClient, externalScreen, ScreenEvent::DISCONNECTED);

    /* step9: inner screen change */
    innerScreenId = innerScreen->GetScreenId();
    externalScreenId = externalScreen->GetScreenId();
    auto rsSetScreenPowerStatusTask = [innerScreenId, externalScreenId, this]() {
        ScreenSessionManager::GetInstance().CallRsSetScreenPowerStatusSync(externalScreenId,
            ScreenPowerStatus::POWER_STATUS_OFF);
        CallRsSetScreenPowerStatusSyncToOn(innerScreenId);
    };
    ScreenSessionManager::GetInstance().GetPowerTaskScheduler()->PostTask(rsSetScreenPowerStatusTask,
        "rsInterface_.SetScreenPowerStatus task");
    TLOGW(WmsLogTag::DMS, "inner main and external extend to external main end.");
    return DMError::DM_OK;
}

/* inner main and external mirror to external main */
DMError MultiScreenPowerChangeManager::HandleInnerMainExternalMirrorChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "%s", __func__);
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker("InnerMainExternalMirrorChange");
    TLOGW(WmsLogTag::DMS, "inner main and external mirror to external main start.");
    std::ostringstream oss;
    oss << "inner screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", combination: " << static_cast<int32_t>(innerScreen->GetScreenCombination())
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId()
        << ", combination: " << static_cast<int32_t>(externalScreen->GetScreenCombination());
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    /* step1: make sure client has extend session */
    bool changeRet = ssmClient->OnMainDisplayNodeChange(innerScreen->GetScreenId(), externalScreen->GetScreenId(),
        externalScreen->GetRSScreenId());
    if (!changeRet) {
        TLOGE(WmsLogTag::DMS, "main_extend OnMainDisplayNodeChange failed.");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    /* step2: set combination */
    MultiScreenChangeUtils::ScreenCombinationChange(innerScreen, externalScreen, ScreenCombination::SCREEN_MIRROR);

    /* step3: change physical screen info between inner and external screen */
    MultiScreenChangeUtils::ScreenPhysicalInfoChange(innerScreen, externalScreen);

    /* step4: change position */
    MultiScreenChangeUtils::ScreenMainPositionChange(externalScreen, innerScreen);

    /* step5: notify external screen property change */
    MultiScreenChangeUtils::ScreenPropertyChangeNotify(innerScreen, externalScreen);

    /* step6: dpi change */
    MultiScreenChangeUtils::ScreenDensityChangeNotify(innerScreen, nullptr);

    /* step7: set screen unavailable */
    MultiScreenChangeUtils::SetScreenAvailableStatus(externalScreen, false);

    /* step8: inner screen power change */
    ScreenSessionManager::GetInstance().CallRsSetScreenPowerStatusSync(externalScreen->GetRSScreenId(),
        ScreenPowerStatus::POWER_STATUS_OFF);
    CallRsSetScreenPowerStatusSyncToOn(innerScreen->GetRSScreenId());
    TLOGW(WmsLogTag::DMS, "inner main and external mirror to external main end.");
    return DMError::DM_OK;
}

/* inner extend and external main to external main */
DMError MultiScreenPowerChangeManager::HandleInnerExtendExternalMainChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }
    TLOGW(WmsLogTag::DMS, "inner extend and external main to external main start.");
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker("InnerExtendExternalMainChange");

    /* step1: disconnect external screen */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "%s", __func__);
    MultiScreenChangeUtils::ScreenConnectionChange(ssmClient, innerScreen, ScreenEvent::DISCONNECTED);

    /* step2: set screen combination */
    MultiScreenChangeUtils::ScreenCombinationChange(externalScreen, innerScreen, ScreenCombination::SCREEN_EXTEND);

    /* step3: set screen unavailable */
    MultiScreenChangeUtils::SetScreenAvailableStatus(innerScreen, false);

    /* step4: screen power change */
    ScreenSessionManager::GetInstance().CallRsSetScreenPowerStatusSync(innerScreen->GetRSScreenId(),
        ScreenPowerStatus::POWER_STATUS_OFF);
    CallRsSetScreenPowerStatusSyncToOn(externalScreen->GetRSScreenId());
    TLOGW(WmsLogTag::DMS, "inner extend and external main to external main end.");
    return DMError::DM_OK;
}

/* inner mirror and external main to external main */
DMError MultiScreenPowerChangeManager::HandleInnerMirrorExternalMainChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "%s", __func__);
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }
    TLOGW(WmsLogTag::DMS, "inner mirror and external main to external main start.");
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker("InnerMirrorExternalMainChange");

    /* step1: notify client screen disconnect. */
    MultiScreenChangeUtils::ScreenConnectionChange(ssmClient, innerScreen, ScreenEvent::DISCONNECTED);

    /* step2: set screen combination. */
    MultiScreenChangeUtils::ScreenCombinationChange(externalScreen, innerScreen, ScreenCombination::SCREEN_MIRROR);

    /* step3: set inner screen unavailable. */
    MultiScreenChangeUtils::SetScreenAvailableStatus(innerScreen, false);

    /* step4: remove inner screen displayNode. */
    ScreenDisplayNodeRemove(innerScreen);

    /* step5: screen power change. */
    ScreenSessionManager::GetInstance().CallRsSetScreenPowerStatusSync(innerScreen->GetRSScreenId(),
        ScreenPowerStatus::POWER_STATUS_OFF);
    CallRsSetScreenPowerStatusSyncToOn(externalScreen->GetRSScreenId());
    TLOGW(WmsLogTag::DMS, "inner mirror and external main to external main end.");
    return DMError::DM_OK;
}

/* recovery external main to old status */
DMError MultiScreenPowerChangeManager::OnRecoveryScreenModeByPowerChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    DMError handleRet = DMError::DM_OK;
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "parameters nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }

    std::ostringstream oss;
    oss << "inner screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", innerCombination_: " << static_cast<int32_t>(innerCombination_)
        << ", isInternal: " << (innerScreen->GetIsInternal() ? "true" : "false")
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId()
        << ", externalCombination_: " << static_cast<int32_t>(externalCombination_)
        << ", isInternal: " << (externalScreen->GetIsInternal() ? "true" : "false");
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    auto recoveryModeHandleCall = handleRecoveryScreenModeChangeMap_.find({innerCombination_, externalCombination_});
    if (recoveryModeHandleCall != handleRecoveryScreenModeChangeMap_.end()) {
        auto recoveryModeHandleFunction = recoveryModeHandleCall->second;
        handleRet = recoveryModeHandleFunction(innerScreen, externalScreen);
        MultiScreenChangeUtils::SetExternalScreenOffScreenRendering(innerScreen, externalScreen);
    } else {
        TLOGE(WmsLogTag::DMS, "failed to find recovery function handler!");
        handleRet = DMError::DM_ERROR_INVALID_CALLING;
    }
    return handleRet;
}

/* recovery external main to inner main and external extend by power on */
DMError MultiScreenPowerChangeManager::HandleRecoveryInnerMainExternalExtendChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }

    std::ostringstream oss;
    oss << "inner screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", innerCombination_: " << static_cast<int32_t>(innerCombination_)
        << ", isInternal: " << (innerScreen->GetIsInternal() ? "true" : "false")
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId()
        << ", externalCombination_: " << static_cast<int32_t>(externalCombination_)
        << ", isInternal: " << (externalScreen->GetIsInternal() ? "true" : "false");
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker("RecoveryInnerMainExternalExtendChange");

    /* step1: create external screen mirror displayNode */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "%s", __func__);
    CreateExternalScreenDisplayNodeOnly(innerScreen, externalScreen, ScreenCombination::SCREEN_EXTEND);
    bool changeRet = ssmClient->OnCreateScreenSessionOnly(innerScreen->GetScreenId(), innerScreen->GetRSScreenId(),
        innerScreen->GetName(), innerScreen->GetIsExtend());
    if (!changeRet) {
        TLOGE(WmsLogTag::DMS, "create screenSession failed.");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    /* step2: change display Node */
    changeRet = ssmClient->OnExtendDisplayNodeChange(innerScreen->GetScreenId(), externalScreen->GetScreenId());
    if (!changeRet) {
        TLOGE(WmsLogTag::DMS, "extend displayNode change failed.");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    /* step3: change combination */
    MultiScreenChangeUtils::ScreenCombinationChange(externalScreen, innerScreen, ScreenCombination::SCREEN_EXTEND);

    /* step4: change physical screen info between inner and external screen */
    MultiScreenChangeUtils::ScreenPhysicalInfoChange(innerScreen, externalScreen);

    /* step5: notify extend change */
    ScreenToExtendChange(ssmClient, innerScreen);

    /* step6: notify external screen property change */
    MultiScreenChangeUtils::ScreenPropertyChangeNotify(innerScreen, externalScreen);

    /* step7: dpi change */
    MultiScreenChangeUtils::ScreenDensityChangeNotify(innerScreen, nullptr);

    /* step8: change position */
    MultiScreenChangeUtils::ScreenExtendPositionChange(innerScreen, externalScreen);

    /* step9: set screen unavailable */
    MultiScreenChangeUtils::SetScreenAvailableStatus(innerScreen, true);

    /* step10: inner screen power on */
    CallRsSetScreenPowerStatusSyncToOn(SCREEN_ID_FULL);
    return DMError::DM_OK;
}

/* recovery external main to inner main and external mirror */
DMError MultiScreenPowerChangeManager::HandleRecoveryInnerMainExternalMirrorChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker("RecoveryInnerMainExternalMirrorChange");

    /* step1: create external screen mirror displayNode */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "%s", __func__);
    CreateExternalScreenDisplayNodeOnly(innerScreen, externalScreen, ScreenCombination::SCREEN_MIRROR);

    /* step2: make sure client has extend session */
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }
    bool changeRet = ssmClient->OnMainDisplayNodeChange(externalScreen->GetScreenId(), innerScreen->GetScreenId(),
        innerScreen->GetRSScreenId());
    if (!changeRet) {
        TLOGE(WmsLogTag::DMS, "call main displayNode change failed.");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    /* step3: set screen combination */
    MultiScreenChangeUtils::ScreenCombinationChange(externalScreen, innerScreen, ScreenCombination::SCREEN_MIRROR);

    /* step4: change physical screen info between inner and external screen */
    MultiScreenChangeUtils::ScreenPhysicalInfoChange(innerScreen, externalScreen);

    /* step5: change position */
    MultiScreenChangeUtils::ScreenMainPositionChange(externalScreen, innerScreen);

    /* step6: notify external screen property change */
    MultiScreenChangeUtils::ScreenPropertyChangeNotify(innerScreen, externalScreen);

    /* step7: dpi change */
    MultiScreenChangeUtils::ScreenDensityChangeNotify(innerScreen, nullptr);

    /* step8: set screen unavailable */
    MultiScreenChangeUtils::SetScreenAvailableStatus(innerScreen, true);

    /* step9: inner screen power on */
    CallRsSetScreenPowerStatusSyncToOn(SCREEN_ID_FULL);
    TLOGW(WmsLogTag::DMS, "recovery external main to inner main and external mirror end.");
    return DMError::DM_OK;
}

/* recovery external main to inner extend and external main */
DMError MultiScreenPowerChangeManager::HandleRecoveryInnerExtendExternalMainChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }
    TLOGW(WmsLogTag::DMS, "recovery external main to inner extend and external main start.");
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker("RecoveryInnerExtendExternalMainChange");

    /* step1: create external screen extend displayNode */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "%s", __func__);
    CreateExternalScreenDisplayNodeOnly(innerScreen, externalScreen, ScreenCombination::SCREEN_EXTEND);
    bool changeRet = ssmClient->OnCreateScreenSessionOnly(innerScreen->GetScreenId(), innerScreen->GetRSScreenId(),
        innerScreen->GetName(), innerScreen->GetIsExtend());
    if (!changeRet) {
        TLOGE(WmsLogTag::DMS, "create screenSession failed.");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    /* step2: change screen combination */
    MultiScreenChangeUtils::ScreenCombinationChange(externalScreen, innerScreen, ScreenCombination::SCREEN_EXTEND);

    /* step3: change position. */
    MultiScreenChangeUtils::ScreenExtendPositionChange(externalScreen, innerScreen);

    /* step4: set screen unavailable. */
    MultiScreenChangeUtils::SetScreenAvailableStatus(innerScreen, true);

    /* step5: change screen to extend. */
    ScreenToExtendChange(ssmClient, innerScreen);

    /* step6: inner screen power on */
    CallRsSetScreenPowerStatusSyncToOn(SCREEN_ID_FULL);
    TLOGW(WmsLogTag::DMS, "recovery external main to inner extend and external main end.");
    return DMError::DM_OK;
}

/* recovery external main to inner mirror and external main */
DMError MultiScreenPowerChangeManager::HandleRecoveryInnerMirrorExternalMainChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    /* step1: create inner screen mirror displayNode */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "%s", __func__);
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker("RecoveryInnerMirrorExternalMainChange");
    TLOGW(WmsLogTag::DMS, "recovery external main to inner mirror and external main start.");
    CreateExternalScreenDisplayNodeOnly(innerScreen, externalScreen, ScreenCombination::SCREEN_MIRROR);

    /* step2: set screen combination */
    MultiScreenChangeUtils::ScreenCombinationChange(externalScreen, innerScreen, ScreenCombination::SCREEN_MIRROR);

    /* step3: set inner screen unavailable */
    MultiScreenChangeUtils::SetScreenAvailableStatus(innerScreen, true);

    /* step4: inner screen power on */
    CallRsSetScreenPowerStatusSyncToOn(SCREEN_ID_FULL);
    TLOGW(WmsLogTag::DMS, "recovery external main to inner mirror and external main end.");

    return DMError::DM_OK;
}

void MultiScreenPowerChangeManager::SetInnerAndExternalCombination(ScreenCombination innerCombination,
    ScreenCombination externalCombination)
{
    innerCombination_ = innerCombination;
    externalCombination_ = externalCombination;
}

void MultiScreenPowerChangeManager::CallRsSetScreenPowerStatusSyncToOn(ScreenId screenId)
{
    if (!PowerMgr::PowerMgrClient::GetInstance().IsFoldScreenOn()) {
        TLOGI(WmsLogTag::DMS, "power state IsFoldScreenOn is false");
        return;
    }
    TLOGI(WmsLogTag::DMS, "power state IsFoldScreenOn is true");
    ScreenSessionManager::GetInstance().CallRsSetScreenPowerStatusSync(screenId,
        ScreenPowerStatus::POWER_STATUS_ON);
}

MultiScreenPowerChangeManager::MultiScreenPowerChangeManager()
{
    /* init screen power change map */
    InitMultiScreenPowerChangeMap();

    /* init screen off change map */
    InitMultiScreenModeOffChangeMap();

    /* init recovery screen mode map */
    InitRecoveryMultiScreenModeChangeMap();
    TLOGW(WmsLogTag::DMS, "init multi screen power change map.");
}

MultiScreenPowerChangeManager::~MultiScreenPowerChangeManager()
{
    handleScreenPowerChangeMap_.clear();
    handleRecoveryScreenModeChangeMap_.clear();
    handleScreenOffModeChangeMap_.clear();
    TLOGW(WmsLogTag::DMS, "destructor");
}
} // namespace OHOS::Rosen
