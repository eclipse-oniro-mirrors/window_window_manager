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

#ifndef OHOS_SESSION_MANAGER_SESSION_LISTENER_CONTROLLER_H
#define OHOS_SESSION_MANAGER_SESSION_LISTENER_CONTROLLER_H

#include <mutex>
#include <tuple>
#include <vector>
#include <list>

#include "pixel_map.h"
#ifndef SUPPORT_SCREEN
#define SUPPORT_SCREEN
#endif
#include "mission_listener_interface.h"
#include "ws_common.h"
#include "wm_common.h"
#include "zidl/session_lifecycle_listener_interface.h"
#include "task_scheduler.h"

namespace OHOS {
namespace Rosen {
using ISessionListener = AAFwk::IMissionListener;
class SceneSession;
class SessionListenerController : public std::enable_shared_from_this<SessionListenerController> {
public:
    SessionListenerController() = default;
    SessionListenerController(const std::shared_ptr<TaskScheduler>& taskScheduler);
    ~SessionListenerController() = default;

    WSError AddSessionListener(const sptr<ISessionListener>& listener);

    void DelSessionListener(const sptr<ISessionListener>& listener);

   /*
    * Window Lifecycle
    */
    void NotifySessionLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent event,
        const SessionInfo& sessionInfo, LifeCycleChangeReason reason = LifeCycleChangeReason::DEFAULT);

    void NotifyAppInstanceLifecycleEvent(SessionState state,
        const sptr<SceneSession>& session, LifeCycleChangeReason reason = LifeCycleChangeReason::DEFAULT);

    void NotifySessionTransferToTargetScreenEvent(const SessionInfo& sessionInfo,
        const uint32_t resultCode, const uint64_t fromScreenId, const uint64_t toScreenId,
        LifeCycleChangeReason reason = LifeCycleChangeReason::DEFAULT);

    WMError RegisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener,
        const std::vector<int32_t>& persistentIdList);
        
    WMError RegisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener,
        const std::vector<std::string>& bundleNameList);

    WMError RegisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener,
        const std::string& bundleName, int32_t appIndex, const std::string& appInstanceKey = "");

    WMError UnregisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener);

    bool IsListenerMapByIdSizeReachLimit() const;

    bool IsListenerMapByBundleSizeReachLimit(bool isBundleNameListEmpty) const;

    bool IsListenerMapByAppInstanceSizeReachLimit() const;

    void NotifySessionClosed(const SessionInfo& sessionInfo,
        LifeCycleChangeReason reason = LifeCycleChangeReason::DEFAULT);

    void NotifySessionSnapshotChanged(int32_t persistentId);

    void NotifySessionMovedToFront(int32_t persistentId);

    void NotifySessionFocused(int32_t persistentId);

    void NotifySessionUnfocused(int32_t persistentId);

    void NotifySessionIconChanged(int32_t persistentId, const std::shared_ptr<OHOS::Media::PixelMap>& icon);

    void NotifySessionLabelUpdated(int32_t persistentId);

    void HandleUnInstallApp(const std::list<int32_t>& sessions);

private:
    void OnListenerDied(const wptr<IRemoteObject>& remote);

    void NotifySessionCreated(int32_t persistentId);

    void NotifySessionDestroyed(int32_t persistentId);

    void NotifySessionBackground(int32_t persistentId);

    void NotifyMissionEvent(ISessionLifecycleListener::SessionLifecycleEvent event, int32_t persistentId);

    template<typename F, typename... Args>
    void CallListeners(F func, Args&& ... args);

    class ListenerDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        using ListenerDiedHandler = std::function<void(const wptr<IRemoteObject>&)>;
        explicit ListenerDeathRecipient(ListenerDiedHandler handler) : diedHandler_(std::move(handler)) {}
        ~ListenerDeathRecipient() = default;
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
    struct AppInstanceFilterKey {
        std::string bundleName_;
        int32_t appIndex_ = 0;
        std::string appInstanceKey_ = "";

        bool operator<(const AppInstanceFilterKey& other) const
        {
            return std::tie(bundleName_, appIndex_, appInstanceKey_) <
                std::tie(other.bundleName_, other.appIndex_, other.appInstanceKey_);
        }
    };

    std::mutex listenerLock_;
    std::vector<sptr<ISessionListener>> sessionListeners_;
    sptr<IRemoteObject::DeathRecipient> listenerDeathRecipient_;

   /*
    * Window Lifecycle
    */
    void ConstructPayload(ISessionLifecycleListener::LifecycleEventPayload& payload, const SessionInfo& sessionInfo,
        const uint32_t resultCode = 0, const uint64_t fromScreenId = 0, const uint64_t toScreenId = 0,
        LifeCycleChangeReason reason = LifeCycleChangeReason::DEFAULT);
    void ConstructBatchPayload(std::vector<ISessionLifecycleListener::LifecycleEventPayload>& payloads,
        const std::vector<sptr<SceneSession>>& sessions);
    void OnSessionLifecycleListenerDied(const wptr<IRemoteObject>& remote);
    void RemoveSessionLifecycleListener(const sptr<IRemoteObject>& target);
    template <typename MapType>
    void RemoveListenersFromMap(MapType& listenerMap,
        const std::function<bool(const sptr<ISessionLifecycleListener>&)>& predicate);

    template <typename KeyType, typename MapType>
    void NotifyListeners(const MapType& listenerMap, const KeyType& key,
        const ISessionLifecycleListener::SessionLifecycleEvent event,
        const ISessionLifecycleListener::LifecycleEventPayload& payload);
    void NotifyAppInstanceListeners(const AppInstanceFilterKey& key,
        const ISessionLifecycleListener::LifecycleEventPayload& payload);
    void NotifyAppInstanceListenersByKey(const AppInstanceFilterKey& filterKey,
        const ISessionLifecycleListener::LifecycleEventPayload& payload);
    std::shared_ptr<TaskScheduler> taskScheduler_;
    sptr<IRemoteObject::DeathRecipient> lifecycleListenerDeathRecipient_;
    std::map<int32_t, std::vector<sptr<ISessionLifecycleListener>>> listenerMapById_;
    std::map<std::string, std::vector<sptr<ISessionLifecycleListener>>> listenerMapByBundle_;
    std::map<AppInstanceFilterKey, std::vector<sptr<ISessionLifecycleListener>>> listenerMapByAppInstance_;
    std::vector<sptr<ISessionLifecycleListener>> listenersOfAllBundles_;
};
}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_SESSION_MANAGER_SESSION_LISTENER_CONTROLLER_H
