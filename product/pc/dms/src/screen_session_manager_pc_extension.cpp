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

#include "../include/screen_session_manager_pc_extension.h"
#include <cstdint>
#include <iomanip>
#include "input_manager.h"
#include "screen_session.h"
#include "screen_session_manager.h"

#ifdef DEVICE_STATUS_ENABLE
#include <interaction_manager.h>
#endif // DEVICE_STATUS_ENABLE

using namespace OHOS::Rosen;
using OHOS::sptr;
using namespace PCExtension;
const int32_t PRODUCT_CODE_SIZE = 4;

WM_IMPLEMENT_SINGLE_INSTANCE(ScreenSessionManagerExt)

sptr<ScreenSession> ScreenSessionManagerExt::GetPhysicalScreenSession(ScreenId screenId, ScreenId defScreenId,
    ScreenProperty property)
{
    sptr<ScreenSession> screenSession = nullptr;
    ScreenSessionConfig config;
    config = {
        .screenId = screenId,
        .rsId = screenId,
        .defaultScreenId = defScreenId,
        .property = property,
    };
    screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    return screenSession;
}

void ScreenSessionManagerExt::NotifyCaptureStatusChangedGlobal()
{
    return;
}

void ScreenSessionManagerExt::OnScreenChangeDefault(ScreenId screenId,
    ScreenEvent screenEvent, ScreenChangeReason reason)
{
    std::lock_guard<std::mutex> lock(screenChangeMutex_);
    std::ostringstream oss;
    oss << "OnScreenChange triggered. screenId: " << static_cast<int32_t>(screenId)
        << " screenEvent: " << static_cast<int32_t>(screenEvent);
    screenEventTracker_.RecordEvent(oss.str());
    TLOGW(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " screenEvent: %{public}d",
        screenId, static_cast<int>(screenEvent));
    SetScreenCorrection();
    sptr<ScreenSession> screenSession = GetOrCreateScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    AdaptSuperHorizonalBoot(screenSession, screenId);
    auto physicalScreenSession = GetOrCreatePhysicalScreenSession(screenId);
    if (!physicalScreenSession) {
        TLOGE(WmsLogTag::DMS, "physicalScreenSession is nullptr");
        return;
    }
    OnFoldScreenChange(screenSession);
    if (screenEvent == ScreenEvent::CONNECTED) {
        connectScreenNumber_++;
        DestroyExtendVirtualScreen();
        HandleScreenConnectEvent(screenSession, screenId, screenEvent);
    } else if (screenEvent == ScreenEvent::DISCONNECTED) {
        connectScreenNumber_--;
        HandleScreenDisconnectEvent(screenSession, screenId, screenEvent);
    } else {
        TLOGE(WmsLogTag::DMS, "screenEvent error!");
    }
    NotifyScreenModeChange();
}

void ScreenSessionManagerExt::GetAndMergeEdidInfo(sptr<ScreenSession> screenSession)
{
    ScreenId screenId = screenSession->GetScreenId();
    struct BaseEdid edid;
    if (!GetEdid(screenId, edid)) {
        TLOGE(WmsLogTag::DMS, "get EDID failed.");
        return;
    }
    std::string serialNumber = ConvertEdidToString(edid);
    screenSession->SetSerialNumber(serialNumber);
    if (!edid.displayProductName_.empty()) {
        screenSession->SetName(edid.displayProductName_);
    } else {
        std::string productCodeStr;
        std::string connector = "-";
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setw(PRODUCT_CODE_SIZE) << std::setfill('0') << edid.productCode_;
        productCodeStr = oss.str();
        screenSession->SetName(edid.manufacturerName_ + connector + productCodeStr);
    }
}

void ScreenSessionManagerExt::ScreenConnectionChanged(sptr<ScreenSession>& screenSession,
    ScreenId screenId, ScreenEvent screenEvent, bool phyMirrorEnable)
{
    auto clientProxy = GetClientProxy();
#ifdef FOLD_ABILITY_ENABLE
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        SuperFoldStateManager::GetInstance().RefreshExternalRegion();
    }
    if (foldScreenController_ != nullptr) {
        HandleFoldDeviceScreenConnect(screenId, screenSession, phyMirrorEnable, screenEvent);
        return;
    }
#endif
    if (IsConcurrentUser()) {
        NotifyUserClientProxy(screenSession, screenId, screenEvent);
    } else if (clientProxy) {
        {
            std::unique_lock<std::mutex> lock(displayAddMutex_);
            needWaitAvailableArea_ = true;
        }
        clientProxy->OnScreenConnectionChanged(GetSessionOption(screenSession, screenId), screenEvent);
        sptr<ScreenSession> internalSession = GetInternalScreenSession();
        if (!RecoverRestoredMultiScreenMode(screenSession)) {
            HandleDefaultMultiScreenMode(internalSession, screenSession);
        }
        sptr<ScreenSession> newInternalSession = GetInternalScreenSession();
        if (newInternalSession != nullptr && internalSession != nullptr &&
            internalSession->GetScreenId() != newInternalSession->GetScreenId()) {
            TLOGW(WmsLogTag::DMS, "main screen changed, reset screenSession.");
            screenSession = internalSession;
        }
        SetExtendedScreenFallbackPlan(screenSession->GetScreenId());
    }
}

bool ScreenSessionManagerExt::IsNeedAddInputServiceAbility()
{
    return true;
}