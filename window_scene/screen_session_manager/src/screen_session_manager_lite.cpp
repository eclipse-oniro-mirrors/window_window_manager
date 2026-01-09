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

#include "screen_session_manager/include/screen_session_manager_lite.h"
#include <system_ability_definition.h>
#include <iservice_registry.h>
#include <transaction/rs_interfaces.h>
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
std::recursive_mutex g_instanceMutex;
} // namespace

ScreenSessionManagerLite::~ScreenSessionManagerLite()
{
    TLOGD(WmsLogTag::DMS, "destroy");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    destroyed_ = true;
}

void ScreenSessionManagerLite::ConnectToServer()
{
    {
        std::shared_lock<std::shared_mutex> sharedLock(screenSessionManagerMutex_);
        if (screenSessionManager_) {
            return;
        }
    }

    std::unique_lock<std::shared_mutex> uniqueLock(screenSessionManagerMutex_);
    if (screenSessionManager_) {
        return;
    }
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityMgr) {
        TLOGE(WmsLogTag::DMS, "Failed to get system ability mgr");
        return;
    }

    auto remoteObject = systemAbilityMgr->GetSystemAbility(DISPLAY_MANAGER_SERVICE_SA_ID);
    if (!remoteObject) {
        TLOGE(WmsLogTag::DMS, "Failed to get display manager service");
        return;
    }

    screenSessionManager_ = iface_cast<IScreenSessionManager>(remoteObject);
    if (screenSessionManager_) {
        ssmDeath_ = new (std::nothrow) ScreenSMDeathRecipient();
        if (!ssmDeath_) {
            TLOGE(WmsLogTag::DMS, "Failed to create death Recipient ptr WMSDeathRecipient");
            return;
        }
        if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(ssmDeath_)) {
            TLOGE(WmsLogTag::DMS, "Failed to add death recipient");
            return;
        }
    }
    if (!screenSessionManager_) {
        TLOGE(WmsLogTag::DMS, "Failed to get screen session manager proxy");
        return;
    }
}

ScreenSessionManagerLite& ScreenSessionManagerLite::GetInstance()
{
    std::lock_guard<std::recursive_mutex> lock(g_instanceMutex);
    static ScreenSessionManagerLite* instance = nullptr;
    if (instance == nullptr) {
        instance = new ScreenSessionManagerLite();
    }
    return *instance;
}

DMError ScreenSessionManagerLite::RegisterDisplayManagerAgent(
    const sptr<IDisplayManagerAgent>& displayManagerAgent, DisplayManagerAgentType type)
{
    ConnectToServer();
    if (screenSessionManager_) {
        return screenSessionManager_->RegisterDisplayManagerAgent(displayManagerAgent, type);
    }
    return DMError::DM_ERROR_NULLPTR;
}

DMError ScreenSessionManagerLite::UnregisterDisplayManagerAgent(
    const sptr<IDisplayManagerAgent>& displayManagerAgent, DisplayManagerAgentType type)
{
    ConnectToServer();
    if (screenSessionManager_) {
        return screenSessionManager_->UnregisterDisplayManagerAgent(displayManagerAgent, type);
    }
    return DMError::DM_ERROR_NULLPTR;
}

FoldDisplayMode ScreenSessionManagerLite::GetFoldDisplayMode()
{
    ConnectToServer();
    if (screenSessionManager_) {
        return screenSessionManager_->GetFoldDisplayMode();
    }
    return FoldDisplayMode::UNKNOWN;
}

void ScreenSessionManagerLite::SetFoldDisplayMode(const FoldDisplayMode displayMode)
{
    ConnectToServer();
    if (screenSessionManager_) {
        screenSessionManager_->SetFoldDisplayMode(displayMode);
    }
}

bool ScreenSessionManagerLite::IsFoldable()
{
    ConnectToServer();
    if (screenSessionManager_) {
        return screenSessionManager_->IsFoldable();
    }
    return false;
}

FoldStatus ScreenSessionManagerLite::GetFoldStatus()
{
    ConnectToServer();
    if (screenSessionManager_) {
        return screenSessionManager_->GetFoldStatus();
    }
    return FoldStatus::UNKNOWN;
}

sptr<DisplayInfo> ScreenSessionManagerLite::GetDefaultDisplayInfo()
{
    ConnectToServer();
    if (screenSessionManager_) {
        return screenSessionManager_->GetDefaultDisplayInfo();
    }
    return nullptr;
}

sptr<DisplayInfo> ScreenSessionManagerLite::GetDisplayInfoById(DisplayId displayId)
{
    ConnectToServer();
    if (screenSessionManager_) {
        return screenSessionManager_->GetDisplayInfoById(displayId);
    }
    return nullptr;
}

sptr<CutoutInfo> ScreenSessionManagerLite::GetCutoutInfo(DisplayId displayId)
{
    ConnectToServer();
    if (screenSessionManager_) {
        return screenSessionManager_->GetCutoutInfo(displayId);
    }
    return nullptr;
}

void ScreenSessionManagerLite::Clear()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if ((screenSessionManager_ != nullptr) && (screenSessionManager_->AsObject() != nullptr)) {
        screenSessionManager_->AsObject()->RemoveDeathRecipient(ssmDeath_);
    }
    if (destroyed_) {
        TLOGD(WmsLogTag::DMS, "Already destroyed");
        return;
    }
    screenSessionManager_ = nullptr;
}

void ScreenSMDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    if (wptrDeath == nullptr) {
        TLOGE(WmsLogTag::DMS, "ScreenSMDeathRecipient wptrDeath is null");
        return;
    }

    sptr<IRemoteObject> object = wptrDeath.promote();
    if (!object) {
        TLOGE(WmsLogTag::DMS, "ScreenSMDeathRecipient object is null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "ScreenSessionManagerLite OnRemoteDied");
    ScreenSessionManagerLite::GetInstance().Clear();
}
} // namespace OHOS::Rosen
