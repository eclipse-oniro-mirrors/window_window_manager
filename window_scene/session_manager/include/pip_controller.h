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

#ifndef OHOS_SESSION_MANAGER_PIP_CONTROLLER_H
#define OHOS_SESSION_MANAGER_PIP_CONTROLLER_H

#include <mutex>
#include <memory>
#include "wm_common.h"
#include "zidl/pip_change_listener.h"
#include "task_scheduler.h"

namespace OHOS {
namespace Rosen {
class PipController : public std::enable_shared_from_this<PipController> {
public:
    PipController() = default;
    explicit PipController(const std::shared_ptr<TaskScheduler>& taskScheduler);
    ~PipController() = default;

    bool GetPipDeviceCollaborationPolicy(int32_t screenId);
    WMError SetPipEnableByScreenId(int32_t screenId, bool enabled);
    WMError UnsetPipEnableByScreenId(int32_t screenId);
    WMError RegisterPipChgListenerByScreenId(int32_t screenId, const sptr<IPipChangeListener>& listener);
    WMError UnregisterPipChgListenerByScreenId(int32_t screenId);

    WMError NotifyMulScreenPipStart(DisplayId screenId, int32_t windowId);

    void ClearPipEnabledMap();
    void ClearPipChgListenerMap();

private:
    class PipListenerDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        using ListenerDiedHandler = std::function<void(const wptr<IRemoteObject>&)>;
        explicit PipListenerDeathRecipient(ListenerDiedHandler handler) : diedHandler_(std::move(handler)) {}
        ~PipListenerDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote) final
        {
            if (diedHandler_) {
                diedHandler_(remote);
            }
        }

    private:
        ListenerDiedHandler diedHandler_;
    };

private:
    std::shared_ptr<TaskScheduler> taskScheduler_ {nullptr};

    std::shared_mutex screenPipEnabledMapLock_;
    std::unordered_map<int32_t, bool> screenPipEnabledMap_;
    std::shared_mutex pipChgListenerMapMutex_;
    std::map<int32_t, sptr<IPipChangeListener>> pipChgListenerMap_;

    // unregister listener when third-party process is killed(such as collaboration_service)
    sptr<IRemoteObject::DeathRecipient> pipListenerDeathRecipient_ {nullptr};
    void OnPipChangeListenerDied(const wptr<IRemoteObject>& remote);
    void RemoveListenerAndEnabledFlag(const sptr<IRemoteObject>& target);
};
}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_SESSION_MANAGER_PIP_CONTROLLER_H
