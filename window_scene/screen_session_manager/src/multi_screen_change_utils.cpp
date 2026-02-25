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

#include "multi_screen_change_utils.h"

#include <transaction/rs_transaction.h>

#include "fold_screen_state_internel.h"
#include "fold_screen_controller/super_fold_state_manager.h"
#include "rs_adapter.h"
#include "screen_session_manager.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

void MultiScreenChangeUtils::ScreenPropertyChangeNotify(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen != nullptr) {
        ScreenProperty property = innerScreen->GetScreenProperty();
        property.SetPropertyChangeReason(ScreenPropertyChangeReason::SCREEN_MODE_CHANGE);
        TLOGW(WmsLogTag::DMS, "property change screenId=%{public}" PRIu64, innerScreen->GetScreenId());
        innerScreen->PropertyChange(property, ScreenPropertyChangeReason::CHANGE_MODE);
        ScreenSessionManager::GetInstance().NotifyScreenChanged(innerScreen->ConvertToScreenInfo(),
            ScreenChangeEvent::CHANGE_MODE);
        ScreenSessionManager::GetInstance().NotifyDisplayChanged(innerScreen->ConvertToDisplayInfo(),
            DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
    }
    if (externalScreen != nullptr) {
        ScreenProperty property = externalScreen->GetScreenProperty();
        property.SetPropertyChangeReason(ScreenPropertyChangeReason::SCREEN_MODE_CHANGE);
        TLOGW(WmsLogTag::DMS, "property change screenId=%{public}" PRIu64, externalScreen->GetScreenId());
        externalScreen->PropertyChange(property, ScreenPropertyChangeReason::CHANGE_MODE);
        ScreenSessionManager::GetInstance().NotifyScreenChanged(externalScreen->ConvertToScreenInfo(),
            ScreenChangeEvent::CHANGE_MODE);
        ScreenSessionManager::GetInstance().NotifyDisplayChanged(externalScreen->ConvertToDisplayInfo(),
            DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
    }
}

void MultiScreenChangeUtils::ScreenDensityChangeNotify(sptr<ScreenSession> innerScreen,
    sptr<ScreenSession> externalScreen)
{
    if (innerScreen != nullptr) {
        ScreenSessionManager::GetInstance().SetVirtualPixelRatio(innerScreen->GetScreenId(),
            innerScreen->GetScreenProperty().GetVirtualPixelRatio());
    }
    if (externalScreen != nullptr) {
        ScreenSessionManager::GetInstance().SetVirtualPixelRatio(externalScreen->GetScreenId(),
            externalScreen->GetScreenProperty().GetVirtualPixelRatio());
    }
}

void MultiScreenChangeUtils::ScreenExtendPositionChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession null.");
        return;
    }
    innerScreen->PropertyChange(innerScreen->GetScreenProperty(),
        ScreenPropertyChangeReason::RELATIVE_POSITION_CHANGE);
    externalScreen->PropertyChange(externalScreen->GetScreenProperty(),
        ScreenPropertyChangeReason::RELATIVE_POSITION_CHANGE);
    {
        std::shared_ptr<RSDisplayNode> innerNode = innerScreen->GetDisplayNode();
        std::shared_ptr<RSDisplayNode> externalNode = externalScreen->GetDisplayNode();
        if (innerNode && externalNode) {
            ScreenSessionManager::GetInstance().SetScreenOffset(innerScreen->GetScreenId(),
                innerScreen->GetScreenProperty().GetStartX(),
                innerScreen->GetScreenProperty().GetStartY());
            ScreenSessionManager::GetInstance().SetScreenOffset(externalScreen->GetScreenId(),
                externalScreen->GetScreenProperty().GetStartX(),
                externalScreen->GetScreenProperty().GetStartY());
        } else {
            TLOGW(WmsLogTag::DMS, "DisplayNode is null");
        }
    }
    RSTransactionAdapter::FlushImplicitTransaction(
        {innerScreen->GetRSUIContext(), externalScreen->GetRSUIContext()});
}

void MultiScreenChangeUtils::SetScreenAvailableStatus(sptr<ScreenSession>& screenSession,
    bool isScreenAvailable)
{
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    ScreenId screenId = screenSession->GetScreenId();
    TLOGW(WmsLogTag::DMS, "screenId=%{public}" PRIu64, screenId);
    screenSession->SetScreenAvailableStatus(isScreenAvailable);
    if (isScreenAvailable) {
        ScreenSessionManager::GetInstance().NotifyDisplayCreate(
            screenSession->ConvertToDisplayInfo());
    } else {
        ScreenSessionManager::GetInstance().NotifyDisplayDestroy(screenId);
    }
}

void MultiScreenChangeUtils::ScreenMainPositionChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    innerScreen->SetStartPosition(0, 0);
    innerScreen->SetXYPosition(0, 0);
    innerScreen->PropertyChange(innerScreen->GetScreenProperty(),
        ScreenPropertyChangeReason::RELATIVE_POSITION_CHANGE);
    externalScreen->SetStartPosition(0, 0);
    externalScreen->SetXYPosition(0, 0);
    externalScreen->PropertyChange(externalScreen->GetScreenProperty(),
        ScreenPropertyChangeReason::RELATIVE_POSITION_CHANGE);
    {
        std::shared_ptr<RSDisplayNode> innerNode = innerScreen->GetDisplayNode();
        std::shared_ptr<RSDisplayNode> externalNode = externalScreen->GetDisplayNode();
        if (innerNode && externalNode) {
            ScreenSessionManager::GetInstance().SetScreenOffset(innerScreen->GetScreenId(), 0, 0);
            ScreenSessionManager::GetInstance().SetScreenOffset(externalScreen->GetScreenId(), 0, 0);
        } else {
            TLOGW(WmsLogTag::DMS, "DisplayNode is null");
        }
    }
    RSTransactionAdapter::FlushImplicitTransaction(
        {innerScreen->GetRSUIContext(), externalScreen->GetRSUIContext()});
    ScreenSessionManager::GetInstance().NotifyDisplayChanged(innerScreen->ConvertToDisplayInfo(),
        DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
    ScreenSessionManager::GetInstance().NotifyDisplayChanged(externalScreen->ConvertToDisplayInfo(),
        DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
}

void MultiScreenChangeUtils::SetExternalScreenOffScreenRendering(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen != nullptr) {
        TLOGW(WmsLogTag::DMS, "inner screen offScreen rendering.");
        innerScreen->SetScreenOffScreenRendering();
    }
    if (externalScreen != nullptr) {
        TLOGW(WmsLogTag::DMS, "external screen offScreen rendering.");
        externalScreen->SetScreenOffScreenRendering();
    }
}

void MultiScreenChangeUtils::ScreenCombinationChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen, ScreenCombination externalCombination)
{
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession null.");
        return;
    }
    /* set inner screen local info */
    innerScreen->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    innerScreen->SetIsExtend(false);

    /* set external screen local info */
    externalScreen->SetScreenCombination(externalCombination);
    externalScreen->SetIsExtend(true);

    sptr<IScreenSessionManagerClient> ssmClient = ScreenSessionManager::GetInstance().GetClientProxy();
    if (ssmClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient null");
        return;
    }
    ssmClient->SetScreenCombination(innerScreen->GetScreenId(), externalScreen->GetScreenId(), externalCombination);
}

void MultiScreenChangeUtils::ScreenSerialNumberChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen sessions null.");
        return;
    }
    std::string innerScreenSN = innerScreen->GetSerialNumber();
    std::string externalScreenSN = externalScreen->GetSerialNumber();
    innerScreen->SetSerialNumber(externalScreenSN);
    externalScreen->SetSerialNumber(innerScreenSN);
}

void MultiScreenChangeUtils::ScreenActiveModesChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen sessions null.");
        return;
    }
    std::vector<sptr<SupportedScreenModes>> innerScreenModes = innerScreen->GetScreenModes();
    std::vector<sptr<SupportedScreenModes>> externalScreenModes = externalScreen->GetScreenModes();
    innerScreen->SetScreenModes(externalScreenModes);
    externalScreen->SetScreenModes(innerScreenModes);

    int32_t innerActiveId = innerScreen->GetActiveId();
    int32_t externalActiveId = externalScreen->GetActiveId();
    innerScreen->SetActiveId(externalActiveId);
    externalScreen->SetActiveId(innerActiveId);
}

void MultiScreenChangeUtils::ScreenRSIdChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen sessions null.");
        return;
    }

    ScreenSessionManager::GetInstance().UpdateScreenIdManager(innerScreen, externalScreen);

    ScreenId innerScreenId = innerScreen->GetRSScreenId();
    ScreenId externalScreenId = externalScreen->GetRSScreenId();
    innerScreen->SetRSScreenId(externalScreenId);
    externalScreen->SetRSScreenId(innerScreenId);

    bool isInnerScreenInternal = innerScreen->GetIsInternal();
    bool isexternalScreenInternal = externalScreen->GetIsInternal();
    innerScreen->SetIsInternal(isexternalScreenInternal);
    externalScreen->SetIsInternal(isInnerScreenInternal);
}

void MultiScreenChangeUtils::ScreenNameChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen sessions null.");
        return;
    }
    std::string innerScreenName = innerScreen->GetName();
    std::string externalScreenName = externalScreen->GetName();
    innerScreen->SetName(externalScreenName);
    externalScreen->SetName(innerScreenName);
}

void MultiScreenChangeUtils::ScreenPropertyChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen sessions null.");
        return;
    }
    /* change property from physical session */
    sptr<ScreenSession> innerPhyScreen =
        ScreenSessionManager::GetInstance().GetPhysicalScreenSession(innerScreen->GetRSScreenId());
    sptr<ScreenSession> externalPhyScreen =
        ScreenSessionManager::GetInstance().GetPhysicalScreenSession(externalScreen->GetRSScreenId());

    if (innerPhyScreen == nullptr || externalPhyScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "physicalScreen is null!");
        return;
    }
    std::ostringstream oss;
    oss << "innerPhyScreen screenId:" << innerPhyScreen->GetScreenId()
        << ", rsId:" << innerPhyScreen->GetRSScreenId()
        << ", externalPhyScreen screenId:" << externalPhyScreen->GetRSScreenId()
        << ", rsId:" << externalPhyScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    ScreenProperty innerPhyProperty = innerPhyScreen->GetScreenProperty();
    ScreenProperty externalPhyProperty = externalPhyScreen->GetScreenProperty();
    innerScreen->SetScreenProperty(externalPhyProperty);
    externalScreen->SetScreenProperty(innerPhyProperty);
}

void MultiScreenChangeUtils::ExchangeScreenSupportedRefreshRate(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen sessions null.");
        return;
    }
    /* change screen supported refresh rate from physical session */
    sptr<ScreenSession> innerPhyScreen =
        ScreenSessionManager::GetInstance().GetPhysicalScreenSession(innerScreen->GetRSScreenId());
    sptr<ScreenSession> externalPhyScreen =
        ScreenSessionManager::GetInstance().GetPhysicalScreenSession(externalScreen->GetRSScreenId());

    if (innerPhyScreen == nullptr || externalPhyScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "physicalScreen is null!");
        return;
    }

    std::vector<uint32_t> innerSupportedRefreshRate = innerPhyScreen->GetSupportedRefreshRate();
    std::vector<uint32_t> externalSupportedRefreshRate = externalPhyScreen->GetSupportedRefreshRate();
    innerScreen->SetSupportedRefreshRate(std::move(externalSupportedRefreshRate));
    externalScreen->SetSupportedRefreshRate(std::move(innerSupportedRefreshRate));
}

void MultiScreenChangeUtils::SetScreenNotifyFlag(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen sessions null.");
        return;
    }
    innerScreen->SetIsAvailableAreaNeedNotify(true);
    externalScreen->SetIsAvailableAreaNeedNotify(true);
}

void MultiScreenChangeUtils::ScreenPhysicalInfoChange(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen sessions null.");
        return;
    }
    std::ostringstream oss;
    oss << "before innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", name: " << innerScreen->GetName()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId()
        << ", name: " << externalScreen->GetName();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    /* change screen property */
    ScreenPropertyChange(innerScreen, externalScreen);

    /* change screen supported refresh rate */
    ExchangeScreenSupportedRefreshRate(innerScreen, externalScreen);

    /* change screen rsId */
    ScreenRSIdChange(innerScreen, externalScreen);

    /* change screen name */
    ScreenNameChange(innerScreen, externalScreen);

    /* change serial number */
    ScreenSerialNumberChange(innerScreen, externalScreen);

    /* change active mode */
    ScreenActiveModesChange(innerScreen, externalScreen);

    /* set notify flag */
    SetScreenNotifyFlag(innerScreen, externalScreen);
    oss.str("");
    oss << "after innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", name: " << innerScreen->GetName()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId()
        << ", name: " << externalScreen->GetName();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
}

void MultiScreenChangeUtils::CreateMirrorSession(sptr<ScreenSession>& mainSession,
    sptr<ScreenSession>& screenSession)
{
    if (mainSession == nullptr || screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null.");
        return;
    }
    std::shared_ptr<RSDisplayNode> displayNode = mainSession->GetDisplayNode();
    if (displayNode == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayNode is null.");
        return;
    }
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    screenSession->SetIsExtend(true);
#ifdef FOLD_ABILITY_ENABLE
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        SuperFoldStateManager::GetInstance().RefreshExternalRegion();
    } else {
#endif
        RSDisplayNodeConfig config = { screenSession->rsId_, true, displayNode->GetId() };
        screenSession->ReuseDisplayNode(config);
#ifdef FOLD_ABILITY_ENABLE
    }
#endif
}

void MultiScreenChangeUtils::ScreenConnectionChange(sptr<IScreenSessionManagerClient> ssmClient,
    sptr<ScreenSession>& screenSession, ScreenEvent screenEvent)
{
    if (ssmClient == nullptr || screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "ssmClient or screenSession is null.");
        return;
    }
    SessionOption option = {
        .rsId_ = screenSession->GetRSScreenId(),
        .name_ = screenSession->GetName(),
        .isExtend_ = screenSession->GetIsExtend(),
        .innerName_ = screenSession->GetInnerName(),
        .screenId_ = screenSession->GetScreenId(),
        .supportsFocus_ = screenSession->GetSupportsFocus(),
    };
    ssmClient->OnScreenConnectionChanged(option, screenEvent);
}

void MultiScreenChangeUtils::CreateExtendSession(sptr<ScreenSession>& screenSession)
{
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null.");
        return;
    }
    screenSession->SetIsExtend(true);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    RSDisplayNodeConfig config = { screenSession->rsId_, false, INVALID_NODEID };
    screenSession->ReuseDisplayNode(config);
}

void MultiScreenChangeUtils::SetMultiScreenModeChangeTracker(std::string changeProc)
{
    ScreenSessionManager::GetInstance().SetMultiScreenModeChangeTracker(changeProc);
}
} // Rosen
} // OHOS