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

#include "multi_screen_mode_change_manager.h"

#include <hitrace_meter.h>
#include <transaction/rs_transaction.h>

#include "window_manager_hilog.h"

namespace OHOS::Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(MultiScreenModeChangeManager)
namespace {
const std::string SCREEN_EXTEND = "extend";
const std::string SCREEN_MIRROR = "mirror";
}

void MultiScreenModeChangeManager::InitMultiScreenModeChangeMap()
{
    if (handleMultiScreenModeChangeMap_.size() != 0) {
        TLOGW(WmsLogTag::DMS, "recovery multi screen mode change map has init!");
        return;
    }
    handleMultiScreenModeChangeMap_[{ ScreenCombination::SCREEN_MAIN, ScreenCombination::SCREEN_EXTEND}] =
        [this](sptr<ScreenSession>& innerScreen, ScreenCombination innerTargetCombination,
        sptr<ScreenSession>& externalScreen, ScreenCombination externalTargetCombination) {
            return HandleInnerMainExternalExtendChange(innerScreen, innerTargetCombination,
                externalScreen, externalTargetCombination);
        };

    handleMultiScreenModeChangeMap_[{ ScreenCombination::SCREEN_MAIN, ScreenCombination::SCREEN_MIRROR}] =
        [this](sptr<ScreenSession>& innerScreen, ScreenCombination innerTargetCombination,
        sptr<ScreenSession>& externalScreen, ScreenCombination externalTargetCombination) {
            return HandleInnerMainExternalMirrorChange(innerScreen, innerTargetCombination,
                externalScreen, externalTargetCombination);
        };

    handleMultiScreenModeChangeMap_[{ ScreenCombination::SCREEN_EXTEND, ScreenCombination::SCREEN_MAIN}] =
        [this](sptr<ScreenSession>& innerScreen, ScreenCombination innerTargetCombination,
        sptr<ScreenSession>& externalScreen, ScreenCombination externalTargetCombination) {
            return HandleInnerExtendExternalMainChange(innerScreen, innerTargetCombination,
                externalScreen, externalTargetCombination);
        };

    handleMultiScreenModeChangeMap_[{ ScreenCombination::SCREEN_MIRROR, ScreenCombination::SCREEN_MAIN}] =
        [this](sptr<ScreenSession>& innerScreen, ScreenCombination innerTargetCombination,
        sptr<ScreenSession>& externalScreen, ScreenCombination externalTargetCombination) {
            return HandleInnerMirrorExternalMainChange(innerScreen, innerTargetCombination,
                externalScreen, externalTargetCombination);
        };
}

DMError MultiScreenModeChangeManager::OnMultiScreenModeChangeRequest(
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen, const std::string& operateType)
{
    DMError handleRet = DMError::DM_OK;
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "parameters nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    ScreenCombination innerTargetCombination = ScreenCombination::SCREEN_MAIN;
    ScreenCombination externalTargetCombination = operateType == SCREEN_EXTEND ?
        ScreenCombination::SCREEN_EXTEND : ScreenCombination::SCREEN_MIRROR;

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", current combination: " << static_cast<int32_t>(innerScreen->GetScreenCombination())
        << ", target combination: " << static_cast<int32_t>(innerTargetCombination)
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId()
        << ", current combination: " << static_cast<int32_t>(externalScreen->GetScreenCombination())
        << ", target combination: " << static_cast<int32_t>(externalTargetCombination);
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    auto modeHandleCall = handleMultiScreenModeChangeMap_.find({innerScreen->GetScreenCombination(),
        externalScreen->GetScreenCombination()});
    if (modeHandleCall != handleMultiScreenModeChangeMap_.end()) {
        auto modeHandleFunction = modeHandleCall->second;
        handleRet = modeHandleFunction(innerScreen, innerTargetCombination, externalScreen,
            externalTargetCombination);
    } else {
        TLOGE(WmsLogTag::DMS, "failed to find function handler!");
        handleRet = DMError::DM_ERROR_INVALID_CALLING;
    }
    return handleRet;
}

/* inner main external extend change */
void MultiScreenModeChangeManager::InitMultiScreenInnerMainExternalExtendModeModeChangeMap()
{
    if (handleMultiScreenInnerMainExternalExtendModeChangeMap_.size() != 0) {
        TLOGW(WmsLogTag::DMS, "recovery multi screen inner main external extend mode change map has init!");
        return;
    }
    /* inner main external extend change to inner main external mirror */
    handleMultiScreenInnerMainExternalExtendModeChangeMap_[{
        ScreenCombination::SCREEN_MAIN, ScreenCombination::SCREEN_MIRROR}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerMainExternalExtendToInnerMainExternalMirrorChange(innerScreen, externalScreen);
        };
    /* inner main external extend change to inner extend external main */
    handleMultiScreenInnerMainExternalExtendModeChangeMap_[{
        ScreenCombination::SCREEN_EXTEND, ScreenCombination::SCREEN_MAIN}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerMainExternalExtendToInnerExtendExternalMainChange(innerScreen, externalScreen);
        };
    /* inner main external extend change to inner mirror external main */
    handleMultiScreenInnerMainExternalExtendModeChangeMap_[{
        ScreenCombination::SCREEN_MIRROR, ScreenCombination::SCREEN_MAIN}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerMainExternalExtendToInnerMirrorExternalMainChange(innerScreen, externalScreen);
        };
}

/* inner main external mirror change */
void MultiScreenModeChangeManager::InitMultiScreenInnerMainExternalMirrorModeModeChangeMap()
{
    if (handleMultiScreenInnerMainExternalMirrorModeChangeMap_.size() != 0) {
        TLOGW(WmsLogTag::DMS, "recovery multi screen inner main external mirror mode change map has init!");
        return;
    }
    /* inner main external mirror change to inner main external extend */
    handleMultiScreenInnerMainExternalMirrorModeChangeMap_[{
        ScreenCombination::SCREEN_MAIN, ScreenCombination::SCREEN_EXTEND}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerMainExternalMirrorToInnerMainExternalExtendChange(innerScreen, externalScreen);
        };
    /* inner main external mirror change to inner mirror external main */
    handleMultiScreenInnerMainExternalMirrorModeChangeMap_[{
        ScreenCombination::SCREEN_MIRROR, ScreenCombination::SCREEN_MAIN}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerMainExternalMirrorToInnerMirrorExternalMainChange(innerScreen, externalScreen);
        };
    /* inner main external mirror change to inner extend external main */
    handleMultiScreenInnerMainExternalMirrorModeChangeMap_[{
        ScreenCombination::SCREEN_EXTEND, ScreenCombination::SCREEN_MAIN}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerMainExternalMirrorToInnerExtendExternalMainChange(innerScreen, externalScreen);
        };
}

/* inner extend external main change */
void MultiScreenModeChangeManager::InitMultiScreenInnerExtendExternalMainModeModeChangeMap()
{
    if (handleMultiScreenInnerExtendExternalMainModeChangeMap_.size() != 0) {
        TLOGW(WmsLogTag::DMS, "recovery multi screen inner extend external main mode change map has init!");
        return;
    }
    /* inner extend external main change to inner mirror external main */
    handleMultiScreenInnerExtendExternalMainModeChangeMap_[{
        ScreenCombination::SCREEN_MIRROR, ScreenCombination::SCREEN_MAIN}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerExtendExternalMainToInnerMirrorExternalMainChange(innerScreen, externalScreen);
        };
    /* inner extend external main change to inner main external extend */
    handleMultiScreenInnerExtendExternalMainModeChangeMap_[{
        ScreenCombination::SCREEN_MAIN, ScreenCombination::SCREEN_EXTEND}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerExtendExternalMainToInnerMainExternalExtendChange(innerScreen, externalScreen);
        };
    /* inner extend external main change to inner main external mirror */
    handleMultiScreenInnerExtendExternalMainModeChangeMap_[{
        ScreenCombination::SCREEN_MAIN, ScreenCombination::SCREEN_MIRROR}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerExtendExternalMainToInnerMainExternalMirrorChange(innerScreen, externalScreen);
        };
}

/* inner mirror external main change */
void MultiScreenModeChangeManager::InitMultiScreenInnerMirrorExternalMainModeModeChangeMap()
{
    if (handleMultiScreenInnerMirrorExternalMainModeChangeMap_.size() != 0) {
        TLOGW(WmsLogTag::DMS, "recovery multi screen inner mirror external main mode change map has init!");
        return;
    }
    /* inner mirror external main change to inner extend external main */
    handleMultiScreenInnerMirrorExternalMainModeChangeMap_[{
        ScreenCombination::SCREEN_EXTEND, ScreenCombination::SCREEN_MAIN}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerMirrorExternalMainToInnerExtendExternalMainChange(innerScreen, externalScreen);
        };
    /* inner mirror external main change to inner main external mirror */
    handleMultiScreenInnerMirrorExternalMainModeChangeMap_[{
        ScreenCombination::SCREEN_MAIN, ScreenCombination::SCREEN_MIRROR}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerMirrorExternalMainToInnerMainExternalMirrorChange(innerScreen, externalScreen);
        };
    /* inner mirror external main change to inner main external extend */
    handleMultiScreenInnerMirrorExternalMainModeChangeMap_[{
        ScreenCombination::SCREEN_MAIN, ScreenCombination::SCREEN_EXTEND}] =
        [this](sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen) {
            return HandleInnerMirrorExternalMainToInnerMainExternalExtendChange(innerScreen, externalScreen);
        };
}

DMError MultiScreenModeChangeManager::HandleInnerMainExternalExtendChange(
    sptr<ScreenSession>& innerScreen, ScreenCombination innerTargetCombination,
    sptr<ScreenSession>& externalScreen, ScreenCombination externalTargetCombination)
{
    DMError handleRet = DMError::DM_OK;
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "parameters nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    auto modeHandleCall = handleMultiScreenInnerMainExternalExtendModeChangeMap_.find(
        {innerTargetCombination, externalTargetCombination});
    if (modeHandleCall != handleMultiScreenInnerMainExternalExtendModeChangeMap_.end()) {
        auto modeHandleFunction = modeHandleCall->second;
        handleRet = modeHandleFunction(innerScreen, externalScreen);
        MultiScreenChangeUtils::SetExternalScreenOffScreenRendering(innerScreen, externalScreen);
    } else {
        TLOGE(WmsLogTag::DMS, "failed to find inner main external extend function handler!");
        handleRet = DMError::DM_ERROR_INVALID_CALLING;
    }
    return handleRet;
}

DMError MultiScreenModeChangeManager::HandleInnerMainExternalMirrorChange(
    sptr<ScreenSession>& innerScreen, ScreenCombination innerTargetCombination,
    sptr<ScreenSession>& externalScreen, ScreenCombination externalTargetCombination)
{
    DMError handleRet = DMError::DM_OK;
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "parameters nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    auto modeHandleCall = handleMultiScreenInnerMainExternalMirrorModeChangeMap_.find(
        {innerTargetCombination, externalTargetCombination});
    if (modeHandleCall != handleMultiScreenInnerMainExternalMirrorModeChangeMap_.end()) {
        auto modeHandleFunction = modeHandleCall->second;
        handleRet = modeHandleFunction(innerScreen, externalScreen);
        MultiScreenChangeUtils::SetExternalScreenOffScreenRendering(innerScreen, externalScreen);
    } else {
        TLOGE(WmsLogTag::DMS, "failed to find inner main external mirror function handler!");
        handleRet = DMError::DM_ERROR_INVALID_CALLING;
    }
    return handleRet;
}

DMError MultiScreenModeChangeManager::HandleInnerExtendExternalMainChange(
    sptr<ScreenSession>& innerScreen, ScreenCombination innerTargetCombination,
    sptr<ScreenSession>& externalScreen, ScreenCombination externalTargetCombination)
{
    DMError handleRet = DMError::DM_OK;
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "parameters nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    auto modeHandleCall = handleMultiScreenInnerExtendExternalMainModeChangeMap_.find(
        {innerTargetCombination, externalTargetCombination});
    if (modeHandleCall != handleMultiScreenInnerExtendExternalMainModeChangeMap_.end()) {
        auto modeHandleFunction = modeHandleCall->second;
        handleRet = modeHandleFunction(innerScreen, externalScreen);
        MultiScreenChangeUtils::SetExternalScreenOffScreenRendering(innerScreen, externalScreen);
    } else {
        TLOGE(WmsLogTag::DMS, "failed to find inner extend external main function handler!");
        handleRet = DMError::DM_ERROR_INVALID_CALLING;
    }
    return handleRet;
}

DMError MultiScreenModeChangeManager::HandleInnerMirrorExternalMainChange(
    sptr<ScreenSession>& innerScreen, ScreenCombination innerTargetCombination,
    sptr<ScreenSession>& externalScreen, ScreenCombination externalTargetCombination)
{
    DMError handleRet = DMError::DM_OK;
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "parameters nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    auto modeHandleCall = handleMultiScreenInnerMirrorExternalMainModeChangeMap_.find(
        {innerTargetCombination, externalTargetCombination});
    if (modeHandleCall != handleMultiScreenInnerMirrorExternalMainModeChangeMap_.end()) {
        auto modeHandleFunction = modeHandleCall->second;
        handleRet = modeHandleFunction(innerScreen, externalScreen);
        MultiScreenChangeUtils::SetExternalScreenOffScreenRendering(innerScreen, externalScreen);
    } else {
        TLOGE(WmsLogTag::DMS, "failed to find inner mirror external main function handler!");
        handleRet = DMError::DM_ERROR_INVALID_CALLING;
    }
    return handleRet;
}

void MultiScreenModeChangeManager::NotifyClientCreateExtendSessionOnly(sptr<IScreenSessionManagerClient>& ssmClient,
    sptr<ScreenSession>& screenSession)
{
    if (ssmClient == nullptr || screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayNode/client/mainSession is nullptr.");
        return;
    }
    TLOGW(WmsLogTag::DMS, "notify client create extend session only.");
    MultiScreenChangeUtils::CreateExtendSession(screenSession);

    ssmClient->OnCreateScreenSessionOnly(screenSession->GetScreenId(), screenSession->GetRSScreenId(),
        screenSession->GetName(), screenSession->GetIsExtend());
}

void MultiScreenModeChangeManager::NotifyClientCreateMirrorSessionOnly(sptr<IScreenSessionManagerClient>& ssmClient,
    sptr<ScreenSession>& screenSession, sptr<ScreenSession>& mainSession)
{
    if (ssmClient == nullptr || screenSession == nullptr ||
        mainSession == nullptr || mainSession->GetDisplayNode() == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayNode/client/mainSession is nullptr.");
        return;
    }
    TLOGW(WmsLogTag::DMS, "notify client create mirror session only.");
    MultiScreenChangeUtils::CreateMirrorSession(mainSession, screenSession);

    ssmClient->OnCreateScreenSessionOnly(screenSession->GetScreenId(), screenSession->GetRSScreenId(),
        screenSession->GetName(), screenSession->GetIsExtend());
}

void MultiScreenModeChangeManager::ScreenChangeToMirrorMode(sptr<IScreenSessionManagerClient> ssmClient,
    sptr<ScreenSession>& screenSession, sptr<ScreenSession>& mainSession)
{
    if (ssmClient == nullptr || screenSession == nullptr || mainSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayNode/client is nullptr.");
        return;
    }

    /* step1: notify client screen disconnect */
    MultiScreenChangeUtils::ScreenConnectionChange(ssmClient, screenSession, ScreenEvent::DISCONNECTED);

    /* step2: create external screen mirror */
    MultiScreenChangeUtils::CreateMirrorSession(mainSession, screenSession);
}

void MultiScreenModeChangeManager::ScreenChangeToExtendMode(sptr<IScreenSessionManagerClient> ssmClient,
    sptr<ScreenSession>& screenSession)
{
    /* step1: create external screen mirror */
    MultiScreenChangeUtils::CreateExtendSession(screenSession);

    /* step2: notify client external screen connect */
    MultiScreenChangeUtils::ScreenConnectionChange(ssmClient, screenSession, ScreenEvent::CONNECTED);
}

DMError MultiScreenModeChangeManager::ScreenDisplayNodeChangeNotify(sptr<IScreenSessionManagerClient> ssmClient,
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
    ScreenId innerScreenId = innerScreen->GetScreenId();
    ScreenId externalScreenId = externalScreen->GetScreenId();

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreenId
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreenId
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    /* step1: scb client change display node */
    bool changeRet = ssmClient->OnExtendDisplayNodeChange(innerScreenId, externalScreenId);
    if (!changeRet) {
        TLOGE(WmsLogTag::DMS, "client display node change failed.");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }
    return DMError::DM_OK;
}

/* inner main external extend change to inner main external mirror */
DMError MultiScreenModeChangeManager::HandleInnerMainExternalExtendToInnerMainExternalMirrorChange(
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
    /* step1: check current combination */
    if (innerScreen->GetScreenCombination() == ScreenCombination::SCREEN_MAIN &&
        externalScreen->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
        TLOGE(WmsLogTag::DMS, "current combination not change.");
        return DMError::DM_OK;
    }
    std::string trackInfo = "inner main external extend change to inner main external mirror";
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker(trackInfo);

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    /* step2: create external screen mirror */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "modeChange[%s]", trackInfo.c_str());
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }
    ScreenChangeToMirrorMode(ssmClient, externalScreen, innerScreen);

    /* step3: change inner screen combination */
    MultiScreenChangeUtils::ScreenCombinationChange(innerScreen, externalScreen, ScreenCombination::SCREEN_MIRROR);

    /* step4: set inner screen position */
    MultiScreenChangeUtils::ScreenMainPositionChange(innerScreen, externalScreen);
    TLOGW(WmsLogTag::DMS, "inner main external extend to inner main external mirror end.");
    return DMError::DM_OK;
}

/* inner main external extend change to inner extend external main */
DMError MultiScreenModeChangeManager::HandleInnerMainExternalExtendToInnerExtendExternalMainChange(
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
    /* step1: check current combination */
    if (innerScreen->GetScreenCombination() == ScreenCombination::SCREEN_EXTEND &&
        externalScreen->GetScreenCombination() == ScreenCombination::SCREEN_MAIN) {
        TLOGE(WmsLogTag::DMS, "current combination not change.");
        return DMError::DM_OK;
    }
    std::string trackInfo = "inner main external extend change to inner extend external main";
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker(trackInfo);

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }

    /* step2: scb client change display node */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "modeChange[%s]", trackInfo.c_str());
    DMError displayNodeChangeRet = ScreenDisplayNodeChangeNotify(ssmClient, innerScreen, externalScreen);
    if (displayNodeChangeRet != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "change displayNode failed.");
        return displayNodeChangeRet;
    }

    /* step3: change physical screen info between inner and external screen */
    MultiScreenChangeUtils::ScreenPhysicalInfoChange(innerScreen, externalScreen);

    /* step4: change position */
    MultiScreenChangeUtils::ScreenExtendPositionChange(innerScreen, externalScreen);

    /* step5: notify external screen property change */
    MultiScreenChangeUtils::ScreenPropertyChangeNotify(innerScreen, externalScreen);

    /* step6: dpi change*/
    MultiScreenChangeUtils::ScreenDensityChangeNotify(innerScreen, externalScreen);

    /* step7: screen combination change*/
    MultiScreenChangeUtils::ScreenCombinationChange(innerScreen, externalScreen, ScreenCombination::SCREEN_EXTEND);

    return DMError::DM_OK;
}

/* inner main external extend change to inner mirror external main */
DMError MultiScreenModeChangeManager::HandleInnerMainExternalExtendToInnerMirrorExternalMainChange(
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
    /* step1: check current combination */
    if (innerScreen->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR &&
        externalScreen->GetScreenCombination() == ScreenCombination::SCREEN_MAIN) {
        TLOGE(WmsLogTag::DMS, "current combination not change.");
        return DMError::DM_OK;
    }
    std::string trackInfo = "inner main external extend change to inner mirror external main";
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker(trackInfo);

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }

    /* step2: scb client change display node */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "modeChange[%s]", trackInfo.c_str());
    DMError displayNodeChangeRet = ScreenDisplayNodeChangeNotify(ssmClient, innerScreen, externalScreen);
    if (displayNodeChangeRet != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "change displayNode failed.");
        return displayNodeChangeRet;
    }

    /* step3: change physical screen info between inner and external screen */
    MultiScreenChangeUtils::ScreenPhysicalInfoChange(innerScreen, externalScreen);

    /* step4: change position */
    MultiScreenChangeUtils::ScreenMainPositionChange(innerScreen, externalScreen);

    /* step5: notify external screen property change */
    MultiScreenChangeUtils::ScreenPropertyChangeNotify(innerScreen, externalScreen);

    /* step6: dpi change */
    MultiScreenChangeUtils::ScreenDensityChangeNotify(innerScreen, externalScreen);

    /* step7: notify client external screen disconnect and create mirror node */
    ScreenChangeToMirrorMode(ssmClient, externalScreen, innerScreen);

    /* step8: screen combination change*/
    MultiScreenChangeUtils::ScreenCombinationChange(innerScreen, externalScreen, ScreenCombination::SCREEN_MIRROR);

    return DMError::DM_OK;
}

/* inner main external mirror change to inner main external extend */
DMError MultiScreenModeChangeManager::HandleInnerMainExternalMirrorToInnerMainExternalExtendChange(
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
    /* step1: check current combination */
    if (innerScreen->GetScreenCombination() == ScreenCombination::SCREEN_MAIN &&
        externalScreen->GetScreenCombination() == ScreenCombination::SCREEN_EXTEND) {
        TLOGE(WmsLogTag::DMS, "current combination not change.");
        return DMError::DM_OK;
    }
    std::string trackInfo = "inner main external mirror change to inner main external extend";
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker(trackInfo);

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    /* step2: change external screen from mirror to extend */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "modeChange[%s]", trackInfo.c_str());
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }
    ScreenChangeToExtendMode(ssmClient, externalScreen);

    /* step3: set inner screen combination */
    MultiScreenChangeUtils::ScreenCombinationChange(innerScreen, externalScreen, ScreenCombination::SCREEN_EXTEND);

    /* step4: set relative position */
    MultiScreenChangeUtils::ScreenExtendPositionChange(innerScreen, externalScreen);
    return DMError::DM_OK;
}

/* inner main external mirror change to inner mirror external main */
DMError MultiScreenModeChangeManager::HandleInnerMainExternalMirrorToInnerMirrorExternalMainChange(
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
    /* step1: check current combination */
    if (innerScreen->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR &&
        externalScreen->GetScreenCombination() == ScreenCombination::SCREEN_MAIN) {
        TLOGE(WmsLogTag::DMS, "current combination not change.");
        return DMError::DM_OK;
    }
    std::string trackInfo = "inner main external mirror change to inner mirror external main";
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker(trackInfo);

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "modeChange[%s]", trackInfo.c_str());
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }
    /* step2: change external mirror to extend mode */
    NotifyClientCreateMirrorSessionOnly(ssmClient, externalScreen, innerScreen);

    /* step3: notify scb change displayNode */
    DMError displayNodeChangeRet = ScreenDisplayNodeChangeNotify(ssmClient, innerScreen, externalScreen);
    if (displayNodeChangeRet != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "change displayNode failed.");
        return displayNodeChangeRet;
    }

    /* step4: change physical screen info between inner and external screen */
    MultiScreenChangeUtils::ScreenPhysicalInfoChange(innerScreen, externalScreen);

    /* step5: change position */
    MultiScreenChangeUtils::ScreenMainPositionChange(innerScreen, externalScreen);

    /* step6: notify external screen property change */
    MultiScreenChangeUtils::ScreenPropertyChangeNotify(innerScreen, externalScreen);

    /* step7: dpi change */
    MultiScreenChangeUtils::ScreenDensityChangeNotify(innerScreen, externalScreen);

    /* step8: screen combination change */
    MultiScreenChangeUtils::ScreenCombinationChange(innerScreen, externalScreen, ScreenCombination::SCREEN_MIRROR);
    return DMError::DM_OK;
}

/* inner main external mirror change to inner extend external main */
DMError MultiScreenModeChangeManager::HandleInnerMainExternalMirrorToInnerExtendExternalMainChange(
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
    /* step1: check current combination */
    if (innerScreen->GetScreenCombination() == ScreenCombination::SCREEN_EXTEND &&
        externalScreen->GetScreenCombination() == ScreenCombination::SCREEN_MAIN) {
        TLOGE(WmsLogTag::DMS, "current combination not change.");
        return DMError::DM_OK;
    }
    std::string trackInfo = "inner main external mirror change to inner extend external main";
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker(trackInfo);

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    /* step2: create screen session for client */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "modeChange[%s]", trackInfo.c_str());
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }

    /* step3: notify client create session only */
    NotifyClientCreateExtendSessionOnly(ssmClient, externalScreen);

    /* step4: notify scb change displayNode */
    DMError displayNodeChangeRet = ScreenDisplayNodeChangeNotify(ssmClient, innerScreen, externalScreen);
    if (displayNodeChangeRet != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "change displayNode failed.");
        return displayNodeChangeRet;
    }

    /* step5: change physical screen info between inner and external screen */
    MultiScreenChangeUtils::ScreenPhysicalInfoChange(innerScreen, externalScreen);

    /* step6: change position */
    MultiScreenChangeUtils::ScreenExtendPositionChange(innerScreen, externalScreen);

    /* step7: notify external screen property change */
    MultiScreenChangeUtils::ScreenPropertyChangeNotify(innerScreen, externalScreen);

    /* step8: dpi change */
    MultiScreenChangeUtils::ScreenDensityChangeNotify(innerScreen, externalScreen);

    /* step9: screen combination change */
    MultiScreenChangeUtils::ScreenCombinationChange(innerScreen, externalScreen,
        ScreenCombination::SCREEN_EXTEND);

    return DMError::DM_OK;
}

/* inner extend external main change to inner mirror external main */
DMError MultiScreenModeChangeManager::HandleInnerExtendExternalMainToInnerMirrorExternalMainChange(
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
    /* step1: check current combination */
    if (innerScreen->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR &&
        externalScreen->GetScreenCombination() == ScreenCombination::SCREEN_MAIN) {
        TLOGE(WmsLogTag::DMS, "current combination not change.");
        return DMError::DM_OK;
    }
    std::string trackInfo = "inner extend external main change to inner mirror external main";
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker(trackInfo);

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    /* step2: notify client screen disconnect */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "modeChange[%s]", trackInfo.c_str());
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }

    /* step3: create external screen mirror */
    ScreenChangeToMirrorMode(ssmClient, innerScreen, externalScreen);

    /* step4: change inner screen combination */
    MultiScreenChangeUtils::ScreenCombinationChange(innerScreen, externalScreen, ScreenCombination::SCREEN_MIRROR);

    /* step5: set inner screen position */
    MultiScreenChangeUtils::ScreenMainPositionChange(externalScreen, innerScreen);
    return DMError::DM_OK;
}

/* inner extend external main change to inner main external extend */
DMError MultiScreenModeChangeManager::HandleInnerExtendExternalMainToInnerMainExternalExtendChange(
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
    /* step1: check current combination */
    if (innerScreen->GetScreenCombination() == ScreenCombination::SCREEN_MAIN &&
        externalScreen->GetScreenCombination() == ScreenCombination::SCREEN_EXTEND) {
        TLOGE(WmsLogTag::DMS, "current combination not change.");
        return DMError::DM_OK;
    }
    std::string trackInfo = "inner extend external main change to inner main external extend";
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker(trackInfo);

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    /* step2: create screenSession for client */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "modeChange[%s]", trackInfo.c_str());
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }

    /* step3: notify scb change displayNode */
    DMError displayNodeChangeRet = ScreenDisplayNodeChangeNotify(ssmClient, innerScreen, externalScreen);
    if (displayNodeChangeRet != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "change displayNode failed.");
        return displayNodeChangeRet;
    }

    /* step4: change physical screen info between inner and external screen */
    MultiScreenChangeUtils::ScreenPhysicalInfoChange(innerScreen, externalScreen);

    /* step5: change position */
    MultiScreenChangeUtils::ScreenExtendPositionChange(innerScreen, externalScreen);

    /* step6: notify external screen property change */
    MultiScreenChangeUtils::ScreenPropertyChangeNotify(innerScreen, externalScreen);

    /* step7: dpi change */
    MultiScreenChangeUtils::ScreenDensityChangeNotify(innerScreen, externalScreen);

    /* step8: screen combination change */
    MultiScreenChangeUtils::ScreenCombinationChange(externalScreen, innerScreen, ScreenCombination::SCREEN_EXTEND);

    return DMError::DM_OK;
}

/* inner extend external main change to inner main external mirror */
DMError MultiScreenModeChangeManager::HandleInnerExtendExternalMainToInnerMainExternalMirrorChange(
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
    /* step1: check current combination */
    if (innerScreen->GetScreenCombination() == ScreenCombination::SCREEN_MAIN &&
        externalScreen->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
        TLOGE(WmsLogTag::DMS, "current combination not change.");
        return DMError::DM_OK;
    }
    std::string trackInfo = "inner extend external main change to inner main external mirror";
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker(trackInfo);

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    /* step2: create screenSession for client */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "modeChange[%s]", trackInfo.c_str());
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }

    /* step3: notify scb change displayNode */
    DMError displayNodeChangeRet = ScreenDisplayNodeChangeNotify(ssmClient, innerScreen, externalScreen);
    if (displayNodeChangeRet != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "change displayNode failed.");
        return displayNodeChangeRet;
    }

    /* step4: change physical screen info between inner and external screen */
    MultiScreenChangeUtils::ScreenPhysicalInfoChange(innerScreen, externalScreen);

    /* step5: change position */
    MultiScreenChangeUtils::ScreenMainPositionChange(externalScreen, innerScreen);

    /* step5: notify external screen property change */
    MultiScreenChangeUtils::ScreenPropertyChangeNotify(innerScreen, externalScreen);

    /* step6: dpi change*/
    MultiScreenChangeUtils::ScreenDensityChangeNotify(innerScreen, externalScreen);

    /* step7: notify client external screen disconnect and create mirror node */
    ScreenChangeToMirrorMode(ssmClient, innerScreen, externalScreen);

    /* step8: change inner screen combination */
    MultiScreenChangeUtils::ScreenCombinationChange(externalScreen, innerScreen, ScreenCombination::SCREEN_MIRROR);

    return DMError::DM_OK;
}

/* inner mirror external main change to inner extend external main */
DMError MultiScreenModeChangeManager::HandleInnerMirrorExternalMainToInnerExtendExternalMainChange(
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
    /* step1: check current combination */
    if (innerScreen->GetScreenCombination() == ScreenCombination::SCREEN_EXTEND &&
        externalScreen->GetScreenCombination() == ScreenCombination::SCREEN_MAIN) {
        TLOGE(WmsLogTag::DMS, "current combination not change.");
        return DMError::DM_OK;
    }
    std::string trackInfo = "inner mirror external main change to inner extend external main";
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker(trackInfo);

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    /* step2: create inner extend mode */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "modeChange[%s]", trackInfo.c_str());
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }
    ScreenChangeToExtendMode(ssmClient, innerScreen);

    /* step3: change external screen combination */
    MultiScreenChangeUtils::ScreenCombinationChange(externalScreen, innerScreen, ScreenCombination::SCREEN_EXTEND);

    /* step4: set inner screen position */
    MultiScreenChangeUtils::ScreenExtendPositionChange(externalScreen, innerScreen);

    return DMError::DM_OK;
}

/* inner mirror external main change to inner main external mirror */
DMError MultiScreenModeChangeManager::HandleInnerMirrorExternalMainToInnerMainExternalMirrorChange(
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
    /* step1: check current combination */
    if (innerScreen->GetScreenCombination() == ScreenCombination::SCREEN_MAIN &&
        externalScreen->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
        TLOGE(WmsLogTag::DMS, "current combination not change.");
        return DMError::DM_OK;
    }
    std::string trackInfo = "inner mirror external main change to inner main external mirror";
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker(trackInfo);

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    /* step2: create client inner session only */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "modeChange[%s]", trackInfo.c_str());
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }
    NotifyClientCreateMirrorSessionOnly(ssmClient, innerScreen, externalScreen);

    /* step3: notify client change displayNode */
    DMError displayNodeChangeRet = ScreenDisplayNodeChangeNotify(ssmClient, innerScreen, externalScreen);
    if (displayNodeChangeRet != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "change displayNode failed.");
        return displayNodeChangeRet;
    }

    /* step4: change physical screen info between inner and external screen */
    MultiScreenChangeUtils::ScreenPhysicalInfoChange(innerScreen, externalScreen);

    /* step5: change position */
    MultiScreenChangeUtils::ScreenMainPositionChange(innerScreen, externalScreen);

    /* step6: notify external screen property change */
    MultiScreenChangeUtils::ScreenPropertyChangeNotify(innerScreen, externalScreen);

    /* step7: dpi change */
    MultiScreenChangeUtils::ScreenDensityChangeNotify(innerScreen, nullptr);

    /* step8: combination change */
    MultiScreenChangeUtils::ScreenCombinationChange(externalScreen, innerScreen, ScreenCombination::SCREEN_MIRROR);

    return DMError::DM_OK;
}

/* inner mirror external main change to inner main external extend */
DMError MultiScreenModeChangeManager::HandleInnerMirrorExternalMainToInnerMainExternalExtendChange(
    sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
    /* step1: check current combination */
    if (innerScreen->GetScreenCombination() == ScreenCombination::SCREEN_MAIN &&
        externalScreen->GetScreenCombination() == ScreenCombination::SCREEN_EXTEND) {
        TLOGE(WmsLogTag::DMS, "current combination not change.");
        return DMError::DM_OK;
    }
    std::string trackInfo = "inner mirror external main change to inner main external extend";
    MultiScreenChangeUtils::SetMultiScreenModeChangeTracker(trackInfo);

    /* step2: create client inner session only */
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "modeChange[%s]", trackInfo.c_str());
    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return DMError::DM_ERROR_NULLPTR;
    }

    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    /* step3: change client inner session only*/
    NotifyClientCreateExtendSessionOnly(ssmClient, innerScreen);

    /* step4: change client and local displayNode */
    DMError displayNodeChangeRet = ScreenDisplayNodeChangeNotify(ssmClient, innerScreen, externalScreen);
    if (displayNodeChangeRet != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "change displayNode failed.");
        return displayNodeChangeRet;
    }

    /* step5: change physical screen info between inner and external screen */
    MultiScreenChangeUtils::ScreenPhysicalInfoChange(innerScreen, externalScreen);

    /* step6: change to extend mode*/
    ScreenChangeToExtendMode(ssmClient, innerScreen);

    /* step7: notify external screen property change */
    MultiScreenChangeUtils::ScreenPropertyChangeNotify(innerScreen, externalScreen);

    /* step8: dpi change */
    MultiScreenChangeUtils::ScreenDensityChangeNotify(innerScreen, externalScreen);

    /* step9: set inner and external screen combination */
    MultiScreenChangeUtils::ScreenCombinationChange(externalScreen, innerScreen, ScreenCombination::SCREEN_EXTEND);

    /* step10: change position */
    MultiScreenChangeUtils::ScreenExtendPositionChange(innerScreen, externalScreen);

    return DMError::DM_OK;
}

MultiScreenModeChangeManager::MultiScreenModeChangeManager()
{
    /* init screen mode change map */
    InitMultiScreenModeChangeMap();

    /* init inner main external extend mode change map */
    InitMultiScreenInnerMainExternalExtendModeModeChangeMap();

    /* init inner main external mirror mode change map */
    InitMultiScreenInnerMainExternalMirrorModeModeChangeMap();

    /* init inner extend external main mode change map */
    InitMultiScreenInnerExtendExternalMainModeModeChangeMap();

    /* init inner mirror external main mode change map */
    InitMultiScreenInnerMirrorExternalMainModeModeChangeMap();

    TLOGW(WmsLogTag::DMS, "init multi screen mode change map.");
}

MultiScreenModeChangeManager::~MultiScreenModeChangeManager()
{
    handleMultiScreenModeChangeMap_.clear();
    handleMultiScreenInnerMainExternalExtendModeChangeMap_.clear();
    handleMultiScreenInnerMainExternalMirrorModeChangeMap_.clear();
    handleMultiScreenInnerExtendExternalMainModeChangeMap_.clear();
    handleMultiScreenInnerMirrorExternalMainModeChangeMap_.clear();
    TLOGI(WmsLogTag::DMS, "destructor");
}
} // namespace OHOS::Rosen