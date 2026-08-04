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
#include "ui_effect_manager.h"
#include "window_manager_hilog.h"
#include "ui_effect_controller_client_proxy.h"

constexpr int CONTROLLER_INDEX = 0;
constexpr int CONTROLLER_CLIENT_INDEX = 1;
constexpr int CONTROLLER_CLIENT_DEATH = 2;
namespace OHOS::Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(UIEffectManager);
void UIEffectManager::RegisterUIEffectSetParamsCallback(NotifyUIEffectSetParamFunc func)
{
    onUIEffectSetParams_ = func;
}

void UIEffectManager::RegisterUIEffectAnimateToCallback(NotifyUIEffectAnimateToFunc func)
{
    onUIEffectAnimateTo_ = func;
}

WMError UIEffectManager::CreateUIEffectController(const sptr<IUIEffectControllerClient>& controllerClient,
    sptr<IUIEffectController>& controller, int32_t& controllerId)
{
    if (!onUIEffectSetParams_ || !onUIEffectAnimateTo_) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "scb not register animation func!");
        return WMError::WM_ERROR_NULLPTR;
    }
    static std::atomic<int32_t> globalControllerId = 0;
    controllerId = ++globalControllerId;
    sptr<UIEffectControllerClientDeath> controllerDeath = sptr<UIEffectControllerClientDeath>::MakeSptr(
        [controllerId, this] () { EraseUIEffectController(controllerId); }
    );
    if (!controllerClient->AsObject() || !controllerClient->AsObject()->AddDeathRecipient(controllerDeath)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to add death recipient");
        return WMError::WM_ERROR_NULLPTR;
    }
    sptr<UIEffectController> newController =
        sptr<UIEffectController>::MakeSptr(controllerId, onUIEffectSetParams_, onUIEffectAnimateTo_);
    if (!newController) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to make sptr newController");
        controllerClient->AsObject()->RemoveDeathRecipient(controllerDeath);
        return WMError::WM_ERROR_NULLPTR;
    }
    std::unique_lock<std::mutex> lock(UIEffectControllerMapMutex_);
    UIEffectControllerMap_[controllerId] = std::make_tuple(newController, controllerClient, controllerDeath);
    controller = newController;
    TLOGI(WmsLogTag::WMS_ANIMATION, "create ui effect success, id: %{public}d, map size %{public}zu", controllerId,
        UIEffectControllerMap_.size());
    return WMError::WM_OK;
}

bool UIEffectManager::IsControllerListValid(const ControllerList& list) const
{
    if (std::get<CONTROLLER_INDEX>(list) != nullptr && std::get<CONTROLLER_CLIENT_INDEX>(list) != nullptr &&
        std::get<CONTROLLER_CLIENT_DEATH>(list) != nullptr) {
        return true;
    } else {
        return false;
    }
}

void UIEffectManager::SetUIEffectControllerAliveState(int32_t id, bool isAlive)
{
    std::unique_lock<std::mutex> lock(UIEffectControllerMapMutex_);
    if (auto it = UIEffectControllerMap_.find(id); it != UIEffectControllerMap_.end()) {
        auto& controllerList = it->second;
        if (!IsControllerListValid(controllerList)) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "controller invalid");
            return;
        }
        sptr<UIEffectController> controller = std::get<CONTROLLER_INDEX>(controllerList);
        controller->SetIsAliveInUI(isAlive);
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "can not find current controller");
    }
}

void UIEffectManager::EraseUIEffectController(int32_t id)
{
    std::unique_lock<std::mutex> lock(UIEffectControllerMapMutex_);
    if (auto it = UIEffectControllerMap_.find(id); it != UIEffectControllerMap_.end()) {
        auto& controllerList = it->second;
        if (!IsControllerListValid(controllerList)) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "controller invalid");
            UIEffectControllerMap_.erase(id);
            return;
        }
        sptr<IUIEffectControllerClient> controllerClient = std::get<CONTROLLER_CLIENT_INDEX>(controllerList);
        sptr<UIEffectControllerClientDeath> clientDeath = std::get<CONTROLLER_CLIENT_DEATH>(controllerList);
        if (!controllerClient->AsObject()) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "controllerClient is nullptr or not proxy");
            UIEffectControllerMap_.erase(id);
            return;
        }
        controllerClient->AsObject()->RemoveDeathRecipient(clientDeath);
    }
    UIEffectControllerMap_.erase(id);
}
}