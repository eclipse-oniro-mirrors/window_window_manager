// /*
//  * Copyright (c) 2023 Huawei Device Co., Ltd.
//  * Licensed under the Apache License, Version 2.0 (the "License");
//  * you may not use this file except in compliance with the License.
//  * You may obtain a copy of the License at,
//  *
//  *     http://www.apache.org/licenses/LICENSE-2.0
//  *
//  * Unless required by applicable law or agreed to in writing, software,
//  * distributed under the License is distributed on an "AS IS" BASIS,
//  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
//  */

#include "display_manager_adapter_lite.h"

#include <iremote_broker.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "display_manager_lite.h"
#include "screen_manager_lite.h"
#include "dm_common.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "DisplayManagerAdapterLite"};
}
WM_IMPLEMENT_SINGLE_INSTANCE(DisplayManagerAdapterLite)
WM_IMPLEMENT_SINGLE_INSTANCE(ScreenManagerAdapterLite)


#define INIT_PROXY_CHECK_RETURN(ret) \
    do { \
        if (!InitDMSProxy()) { \
            WLOGFE("InitDMSProxy fail"); \
            return ret; \
        } \
    } while (false)

DMError BaseAdapterLite::RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->RegisterDisplayManagerAgent(displayManagerAgent, type);
}

DMError BaseAdapterLite::UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->UnregisterDisplayManagerAgent(displayManagerAgent, type);
}

bool BaseAdapterLite::InitDMSProxy()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!isProxyValid_) {
        sptr<ISystemAbilityManager> systemAbilityManager =
                SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityManager) {
            WLOGFE("Failed to get system ability mgr.");
            return false;
        }

        sptr<IRemoteObject> remoteObject
            = systemAbilityManager->GetSystemAbility(DISPLAY_MANAGER_SERVICE_SA_ID);
        if (!remoteObject) {
            WLOGFE("Failed to get display manager service.");
            return false;
        }
        displayManagerServiceProxy_ = new(std::nothrow) DisplayManagerLiteProxy(remoteObject);
        if ((!displayManagerServiceProxy_) || (!displayManagerServiceProxy_->AsObject())) {
            WLOGFW("Failed to get system display manager services");
            return false;
        }

        dmsDeath_ = new(std::nothrow) DMSDeathRecipientLite(*this);
        if (dmsDeath_ == nullptr) {
            WLOGFE("Failed to create death Recipient ptr DMSDeathRecipient");
            return false;
        }
        if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(dmsDeath_)) {
            WLOGFE("Failed to add death recipient");
            return false;
        }
        isProxyValid_ = true;
    }
    return true;
}

sptr<DisplayInfo> DisplayManagerAdapterLite::GetDefaultDisplayInfo()
{
    INIT_PROXY_CHECK_RETURN(nullptr);

    return displayManagerServiceProxy_->GetDefaultDisplayInfo();
}

bool DisplayManagerAdapterLite::IsFoldable()
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->IsFoldable();
}

FoldStatus DisplayManagerAdapterLite::GetFoldStatus()
{
    INIT_PROXY_CHECK_RETURN(FoldStatus::UNKNOWN);

    return displayManagerServiceProxy_->GetFoldStatus();
}

FoldDisplayMode DisplayManagerAdapterLite::GetFoldDisplayMode()
{
    INIT_PROXY_CHECK_RETURN(FoldDisplayMode::UNKNOWN);

    return displayManagerServiceProxy_->GetFoldDisplayMode();
}

void DisplayManagerAdapterLite::SetFoldDisplayMode(const FoldDisplayMode mode)
{
    INIT_PROXY_CHECK_RETURN();

    return displayManagerServiceProxy_->SetFoldDisplayMode(mode);
}

sptr<DisplayInfo> DisplayManagerAdapterLite::GetDisplayInfo(DisplayId displayId)
{
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGFW("screen id is invalid");
        return nullptr;
    }
    INIT_PROXY_CHECK_RETURN(nullptr);

    return displayManagerServiceProxy_->GetDisplayInfoById(displayId);
}

sptr<CutoutInfo> DisplayManagerAdapterLite::GetCutoutInfo(DisplayId displayId)
{
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGFE("screen id is invalid");
        return nullptr;
    }
    INIT_PROXY_CHECK_RETURN(nullptr);
    return displayManagerServiceProxy_->GetCutoutInfo(displayId);
}

/*
 * used by powermgr
 */
bool DisplayManagerAdapterLite::WakeUpBegin(PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->WakeUpBegin(reason);
}

bool DisplayManagerAdapterLite::WakeUpEnd()
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->WakeUpEnd();
}

bool DisplayManagerAdapterLite::SuspendBegin(PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->SuspendBegin(reason);
}

bool DisplayManagerAdapterLite::SuspendEnd()
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->SuspendEnd();
}

ScreenId DisplayManagerAdapterLite::GetInternalScreenId()
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->GetInternalScreenId();
}

bool DisplayManagerAdapterLite::SetScreenPowerById(ScreenId screenId, ScreenPowerState state,
    PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);
    
    return displayManagerServiceProxy_->SetScreenPowerById(screenId, state, reason);
}

bool DisplayManagerAdapterLite::SetDisplayState(DisplayState state)
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->SetDisplayState(state);
}

DisplayState DisplayManagerAdapterLite::GetDisplayState(DisplayId displayId)
{
    INIT_PROXY_CHECK_RETURN(DisplayState::UNKNOWN);

    return displayManagerServiceProxy_->GetDisplayState(displayId);
}

bool DisplayManagerAdapterLite::TryToCancelScreenOff()
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->TryToCancelScreenOff();
}

bool DisplayManagerAdapterLite::SetScreenBrightness(uint64_t screenId, uint32_t level)
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->SetScreenBrightness(screenId, level);
}

uint32_t DisplayManagerAdapterLite::GetScreenBrightness(uint64_t screenId)
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->GetScreenBrightness(screenId);
}

std::vector<DisplayId> DisplayManagerAdapterLite::GetAllDisplayIds()
{
    WLOGFD("DisplayManagerAdapterLite::GetAllDisplayIds enter");
    INIT_PROXY_CHECK_RETURN(std::vector<DisplayId>());

    return displayManagerServiceProxy_->GetAllDisplayIds();
}

VirtualScreenFlag DisplayManagerAdapterLite::GetVirtualScreenFlag(ScreenId screenId)
{
    INIT_PROXY_CHECK_RETURN(VirtualScreenFlag::DEFAULT);
    if (screenId == SCREEN_ID_INVALID) {
        WLOGFE("screenId id is invalid");
        return VirtualScreenFlag::DEFAULT;
    }
    return displayManagerServiceProxy_->GetVirtualScreenFlag(screenId);
}

DMError DisplayManagerAdapterLite::SetSystemKeyboardStatus(bool isOn)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);
    
    return displayManagerServiceProxy_->SetSystemKeyboardStatus(isOn);
}

bool ScreenManagerAdapterLite::SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state,
    PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->SetSpecifiedScreenPower(screenId, state, reason);
}

bool ScreenManagerAdapterLite::SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(false);

    return displayManagerServiceProxy_->SetScreenPowerForAll(state, reason);
}

ScreenPowerState ScreenManagerAdapterLite::GetScreenPower(ScreenId dmsScreenId)
{
    INIT_PROXY_CHECK_RETURN(ScreenPowerState::INVALID_STATE);

    return displayManagerServiceProxy_->GetScreenPower(dmsScreenId);
}

ScreenPowerState ScreenManagerAdapterLite::GetScreenPower()
{
    INIT_PROXY_CHECK_RETURN(ScreenPowerState::INVALID_STATE);

    return displayManagerServiceProxy_->GetScreenPower();
}

DMError ScreenManagerAdapterLite::GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos)
{
    INIT_PROXY_CHECK_RETURN(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED);

    return displayManagerServiceProxy_->GetAllScreenInfos(screenInfos);
}

DMSDeathRecipientLite::DMSDeathRecipientLite(BaseAdapterLite& adapter) : adapter_(adapter)
{
}

void DMSDeathRecipientLite::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    if (wptrDeath == nullptr) {
        WLOGFE("wptrDeath is nullptr");
        return;
    }

    sptr<IRemoteObject> object = wptrDeath.promote();
    if (!object) {
        WLOGFE("object is nullptr");
        return;
    }
    WLOGFI("dms OnRemoteDied");
    adapter_.Clear();
    if (SingletonContainer::IsDestroyed()) {
        WLOGFE("SingletonContainer is destroyed");
        return;
    }
    SingletonContainer::Get<DisplayManagerLite>().OnRemoteDied();
    SingletonContainer::Get<ScreenManagerLite>().OnRemoteDied();
    return;
}

BaseAdapterLite::~BaseAdapterLite()
{
    WLOGFI("destroy");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    Clear();
    displayManagerServiceProxy_ = nullptr;
}

void BaseAdapterLite::Clear()
{
    WLOGFI("Clear");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if ((displayManagerServiceProxy_ != nullptr) && (displayManagerServiceProxy_->AsObject() != nullptr)) {
        displayManagerServiceProxy_->AsObject()->RemoveDeathRecipient(dmsDeath_);
    }
    isProxyValid_ = false;
}
} // namespace OHOS::Rosen