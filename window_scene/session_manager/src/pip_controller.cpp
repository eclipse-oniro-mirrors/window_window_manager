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

#include "pip_controller.h"
#include "window_manager_hilog.h"
#include <cinttypes>

namespace OHOS {
namespace Rosen {

PipController::PipController(const std::shared_ptr<TaskScheduler>& taskScheduler)
{
    taskScheduler_ = taskScheduler;
}

WMError PipController::SetPipEnableByScreenId(int32_t screenId, bool isEnabled)
{
    TLOGI(WmsLogTag::WMS_PIP, "SetPipEnableByScreenId: %{public}d, isEnable: %{public}d", screenId, isEnabled);
    std::unique_lock<std::shared_mutex> lock(screenPipEnabledMapLock_);
    screenPipEnabledMap_.insert_or_assign(screenId, isEnabled);
    return WMError::WM_OK;
}
 
WMError PipController::UnsetPipEnableByScreenId(int32_t screenId)
{
    TLOGI(WmsLogTag::WMS_PIP, "UnsetPipEnableByScreenId: %{public}d", screenId);
    std::unique_lock<std::shared_mutex> lock(screenPipEnabledMapLock_);
    screenPipEnabledMap_.erase(screenId);
    return WMError::WM_OK;
}
 
bool PipController::GetPipDeviceCollaborationPolicy(int32_t screenId)
{
    TLOGI(WmsLogTag::WMS_PIP, "GetPipDeviceCollaborationPolicy: %{public}d", screenId);
    std::shared_lock<std::shared_mutex> lock(screenPipEnabledMapLock_);
    auto iter = screenPipEnabledMap_.find(screenId);
    if (iter == screenPipEnabledMap_.end()) {
        return true;
    }
    return iter->second;
}
 
void PipController::RemoveListenerAndEnabledFlag(const sptr<IRemoteObject>& target)
{
    std::unique_lock<std::shared_mutex> lock(pipChgListenerMapMutex_);
    for (auto it = pipChgListenerMap_.begin(); it != pipChgListenerMap_.end();) {
        auto& listener = it->second;
        if (listener != nullptr && listener->AsObject() == target) {
            TLOGI(WmsLogTag::WMS_PIP, "remove listener");
            UnsetPipEnableByScreenId(it->first);
            it = pipChgListenerMap_.erase(it);
        } else {
            ++it;
        }
    }
}

void PipController::OnPipChangeListenerDied(const wptr<IRemoteObject>& remote)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    taskScheduler_->PostAsyncTask(
        [weakThis = weak_from_this(), remote, where = __func__] {
            auto controller = weakThis.lock();
            if (controller == nullptr) {
                TLOGNE(WmsLogTag::WMS_PIP, "controller is null");
                return;
            }
            auto remoteObj = remote.promote();
            if (!remoteObj) {
                TLOGND(WmsLogTag::WMS_PIP, "invalid remote object");
                return;
            }
            remoteObj->RemoveDeathRecipient(controller->pipListenerDeathRecipient_);
            TLOGI(WmsLogTag::WMS_PIP, "SessionLifecycleListener removed on died");
            controller->RemoveListenerAndEnabledFlag(remoteObj);
        }, __func__);
}

WMError PipController::RegisterPipChgListenerByScreenId(int32_t screenId,
    const sptr<IPipChangeListener>& listener)
{
    TLOGI(WmsLogTag::WMS_PIP, "RegisterPipChgListenerByScreenId: %{public}d", screenId);
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "listener is nullptr");
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    std::unique_lock<std::shared_mutex> lock(pipChgListenerMapMutex_);
    if (!pipListenerDeathRecipient_) {
        auto task = [weakThis = weak_from_this()](const wptr<IRemoteObject>& remote) {
            auto controller = weakThis.lock();
            if (controller != nullptr) {
                controller->OnPipChangeListenerDied(remote);
            }
        };
        pipListenerDeathRecipient_ = sptr<PipListenerDeathRecipient>::MakeSptr(task);
    }
    if (auto listenerObject = listener->AsObject(); listenerObject && listenerObject->IsProxyObject()) {
        listenerObject->AddDeathRecipient(pipListenerDeathRecipient_);
        TLOGD(WmsLogTag::WMS_PIP, "AddDeathRecipient");
    }
    pipChgListenerMap_.insert_or_assign(screenId, listener);
    return WMError::WM_OK;
}
 
WMError PipController::UnregisterPipChgListenerByScreenId(int32_t screenId)
{
    TLOGI(WmsLogTag::WMS_PIP, "UnregisterPipChgListenerByScreenId: %{public}d", screenId);
    std::unique_lock<std::shared_mutex> lock(pipChgListenerMapMutex_);
    pipChgListenerMap_.erase(screenId);
    return WMError::WM_OK;
}

WMError PipController::NotifyMulScreenPipStart(DisplayId screenId, int32_t windowId)
{
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid screenId");
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    TLOGI(WmsLogTag::WMS_PIP, "Notify MulScreen, %{public}d, screenId:%{public}" PRIu64, windowId, screenId);
    sptr<IPipChangeListener> listener = nullptr;
    {
        std::shared_lock<std::shared_mutex> lock(pipChgListenerMapMutex_);
        auto iter = pipChgListenerMap_.find(screenId);
        if (iter != pipChgListenerMap_.end()) {
            listener = iter->second;
        }
    }
    if (listener != nullptr) {
        listener->OnPipStart(windowId);
    }
    return WMError::WM_OK;
}

void PipController::ClearPipEnabledMap()
{
    std::unique_lock<std::shared_mutex> lock(screenPipEnabledMapLock_);
    screenPipEnabledMap_.clear();
}

void PipController::ClearPipChgListenerMap()
{
    std::unique_lock<std::shared_mutex> lock(pipChgListenerMapMutex_);
    pipChgListenerMap_.clear();
}
} // namespace Rosen
} // namespace OHOS
