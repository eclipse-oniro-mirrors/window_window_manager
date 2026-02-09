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

#include "session_listener_controller.h"
#include "scene_session_manager.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "SessionListenerController"};
}
constexpr int32_t MAX_LIFECYCLE_LISTENER_LIMIT = 32;
constexpr int32_t MAX_LISTEN_TARGET_LIMIT = 512;
constexpr int32_t MAX_BUNDLE_NAME_LEN = 128;

SessionListenerController::SessionListenerController(const std::shared_ptr<TaskScheduler>& taskScheduler)
{
    taskScheduler_ = taskScheduler;
}

WSError SessionListenerController::AddSessionListener(const sptr<ISessionListener>& listener)
{
    if (!listener) {
        WLOGFE("listener is invalid");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    std::lock_guard guard(listenerLock_);
    auto it = std::find_if(sessionListeners_.begin(), sessionListeners_.end(),
                           [&listener](const sptr<ISessionListener>& item) {
                               return item && item->AsObject() == listener->AsObject();
                           });
    if (it != sessionListeners_.end()) {
        WLOGFW("listener was already added");
        return WSError::WS_OK;
    }

    if (!listenerDeathRecipient_) {
        auto task = [weakThis = weak_from_this()](const wptr<IRemoteObject>& remote) {
            if (auto controller = weakThis.lock()) {
                controller->OnListenerDied(remote);
            }
        };
        listenerDeathRecipient_ = sptr<ListenerDeathRecipient>::MakeSptr(task);
    }
    if (auto listenerObject = listener->AsObject(); listenerObject && listenerObject->IsProxyObject()) {
        listenerObject->AddDeathRecipient(listenerDeathRecipient_);
    }
    sessionListeners_.emplace_back(listener);
    WLOGFI("success");
    return WSError::WS_OK;
}

void SessionListenerController::DelSessionListener(const sptr<ISessionListener>& listener)
{
    if (!listener) {
        WLOGFE("listener is invalid");
        return;
    }

    std::lock_guard guard(listenerLock_);
    auto it = std::find_if(sessionListeners_.begin(), sessionListeners_.end(),
                           [&listener](const sptr<ISessionListener>& item) {
                               return item && item->AsObject() == listener->AsObject();
                           });
    if (it != sessionListeners_.end()) {
        WLOGFI("success");
        if (auto listenerObject = listener->AsObject(); listenerObject && listenerObject->IsProxyObject()) {
            listenerObject->RemoveDeathRecipient(listenerDeathRecipient_);
        }
        sessionListeners_.erase(it);
    }
}

void SessionListenerController::NotifySessionCreated(int32_t persistentId)
{
    if (persistentId == -1) {
        return;
    }
    WLOGFI("Id:%{public}d", persistentId);
    CallListeners(&ISessionListener::OnMissionCreated, persistentId);
}

void SessionListenerController::NotifySessionDestroyed(int32_t persistentId)
{
    if (persistentId == -1) {
        return;
    }
    WLOGFI("Id:%{public}d", persistentId);
    CallListeners(&ISessionListener::OnMissionDestroyed, persistentId);
}

void SessionListenerController::NotifySessionBackground(int32_t persistentId)
{
    if (persistentId <= INVALID_SESSION_ID) {
        TLOGE(WmsLogTag::WMS_LIFE, "invalid persistentId!");
        return;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Id:%{public}d", persistentId);
    CallListeners(&ISessionListener::OnMissionMovedToBackground, persistentId);
}

void SessionListenerController::HandleUnInstallApp(const std::list<int32_t>& sessions)
{
    for (auto id : sessions) {
        CallListeners(&ISessionListener::OnMissionDestroyed, id);
    }
}

void SessionListenerController::NotifySessionSnapshotChanged(int32_t persistentId)
{
    if (persistentId == -1) {
        return;
    }
    WLOGFI("Id:%{public}d", persistentId);
    CallListeners(&ISessionListener::OnMissionSnapshotChanged, persistentId);
}

void SessionListenerController::NotifySessionMovedToFront(int32_t persistentId)
{
    if (persistentId == -1) {
        return;
    }
    WLOGFI("Id:%{public}d", persistentId);
    CallListeners(&ISessionListener::OnMissionMovedToFront, persistentId);
}

void SessionListenerController::NotifySessionFocused(int32_t persistentId)
{
    if (persistentId == -1) {
        return;
    }

    TLOGI(WmsLogTag::WMS_FOCUS, "Id:%{public}d", persistentId);
    CallListeners(&ISessionListener::OnMissionFocused, persistentId);
}

void SessionListenerController::NotifySessionUnfocused(int32_t persistentId)
{
    if (persistentId == -1) {
        return;
    }

    TLOGI(WmsLogTag::WMS_FOCUS, "Id:%{public}d", persistentId);
    CallListeners(&ISessionListener::OnMissionUnfocused, persistentId);
}

void SessionListenerController::NotifySessionIconChanged(int32_t persistentId,
                                                         const std::shared_ptr<OHOS::Media::PixelMap>& icon)
{
    if (persistentId == -1) {
        return;
    }
    WLOGFI("Id:%{public}d", persistentId);
    CallListeners(&ISessionListener::OnMissionIconUpdated, persistentId, icon);
}

void SessionListenerController::NotifySessionClosed(const SessionInfo& sessionInfo, LifeCycleChangeReason reason)
{
    int32_t persistentId = sessionInfo.persistentId_;
    if (persistentId == -1) {
        return;
    }

    WLOGFI("Id:%{public}d", persistentId);
    CallListeners(&ISessionListener::OnMissionClosed, persistentId);
    NotifySessionLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::DESTROYED, sessionInfo, reason);
}

void SessionListenerController::NotifySessionLabelUpdated(int32_t persistentId)
{
    if (persistentId == -1) {
        return;
    }

    WLOGFI("Id:%{public}d", persistentId);
    CallListeners(&ISessionListener::OnMissionLabelUpdated, persistentId);
}

void SessionListenerController::OnListenerDied(const wptr<IRemoteObject>& remote)
{
    WLOGFD("in");
    auto remoteObj = remote.promote();
    if (!remoteObj) {
        WLOGFD("invalid remote object");
        return;
    }
    remoteObj->RemoveDeathRecipient(listenerDeathRecipient_);

    std::lock_guard guard(listenerLock_);
    auto it = std::find_if(sessionListeners_.begin(), sessionListeners_.end(),
                           [&remoteObj](const sptr<ISessionListener>& item) {
                               return item && item->AsObject() == remoteObj;
                           });
    if (it != sessionListeners_.end()) {
        WLOGFI("SessionListener removed on died");
        sessionListeners_.erase(it);
    }
}

void SessionListenerController::OnSessionLifecycleListenerDied(const wptr<IRemoteObject>& remote)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    taskScheduler_->PostAsyncTask([weakThis = weak_from_this(), remote, where = __func__] {
        auto controller = weakThis.lock();
        if (controller == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "controller is null");
            return;
        }
        auto remoteObj = remote.promote();
        if (!remoteObj) {
            TLOGND(WmsLogTag::WMS_LIFE, "invalid remote object");
            return;
        }
        remoteObj->RemoveDeathRecipient(controller->lifecycleListenerDeathRecipient_);
        TLOGNI(WmsLogTag::WMS_LIFE, "SessionLifecycleListener removed on died.");
        controller->RemoveSessionLifecycleListener(remoteObj);
    }, __func__);
}

template<typename F, typename... Args>
void SessionListenerController::CallListeners(F func, Args&& ... args)
{
    std::vector<sptr<ISessionListener>> sessionListenersTemp;
    {
        std::lock_guard guard(listenerLock_);
        WLOGFD("size:%{public}d", static_cast<int32_t>(sessionListeners_.size()));
        sessionListenersTemp = sessionListeners_;
    }
    for (auto listener : sessionListenersTemp) {
        if (listener) {
            (listener->*func)(std::forward<Args>(args)...);
        }
    }
}

void SessionListenerController::ConstructPayload(ISessionLifecycleListener::LifecycleEventPayload& payload,
    const SessionInfo& sessionInfo, const uint32_t resultCode, const uint64_t fromScreenId, const uint64_t toScreenId,
    LifeCycleChangeReason reason)
{
    payload.bundleName_ = sessionInfo.bundleName_;
    payload.moduleName_ = sessionInfo.moduleName_;
    payload.abilityName_ = sessionInfo.abilityName_;
    payload.appIndex_ = sessionInfo.appIndex_;
    payload.persistentId_ = sessionInfo.persistentId_;
    payload.resultCode_ = resultCode;
    payload.fromScreenId_ = fromScreenId;
    payload.toScreenId_ = toScreenId;
    payload.screenId_ = sessionInfo.screenId_;
    payload.lifeCycleChangeReason_ = reason;
}

WMError SessionListenerController::RegisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener,
    const std::vector<int32_t>& persistentIdList)
{
    if (!listener) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener is invalid.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (!lifecycleListenerDeathRecipient_) {
        auto task = [weakThis = weak_from_this()](const wptr<IRemoteObject>& remote) {
            if (auto controller = weakThis.lock()) {
                controller->OnSessionLifecycleListenerDied(remote);
            }
        };
        lifecycleListenerDeathRecipient_ = sptr<ListenerDeathRecipient>::MakeSptr(task);
    }
    auto listenerObject = listener->AsObject();
    if (listenerObject) {
        listenerObject->AddDeathRecipient(lifecycleListenerDeathRecipient_);
    }
    if (UnregisterSessionLifecycleListener(listener) != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener is invalid.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    bool hasValidId = false;
    for (const int32_t id : persistentIdList) {
        if (!SceneSessionManager::GetInstance().IsMainWindowByPersistentId(id)) {
            TLOGW(WmsLogTag::WMS_LIFE, "invalid persistentId");
            continue;
        }
        hasValidId = true;
        auto it = listenerMapById_.find(id);
        if (it != listenerMapById_.end()) {
            if (it->second.size() >= MAX_LIFECYCLE_LISTENER_LIMIT) {
                TLOGE(WmsLogTag::WMS_LIFE, "The number of listen target has reached the upper limit.");
                return WMError::WM_ERROR_INVALID_OPERATION;
            }
            it->second.emplace_back(listener);
        } else {
            std::vector<sptr<ISessionLifecycleListener>> newListenerList;
            newListenerList.emplace_back(listener);
            listenerMapById_.emplace(id, newListenerList);
        }
        TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d", id);
    }
    return hasValidId ? WMError::WM_OK : WMError::WM_ERROR_INVALID_PARAM;
}

WMError SessionListenerController::RegisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener,
    const std::vector<std::string>& bundleNameList)
{
    if (!listener) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener is invalid.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (!lifecycleListenerDeathRecipient_) {
        auto task = [weakThis = weak_from_this()](const wptr<IRemoteObject>& remote) {
            if (auto controller = weakThis.lock()) {
                controller->OnSessionLifecycleListenerDied(remote);
            }
        };
        lifecycleListenerDeathRecipient_ = sptr<ListenerDeathRecipient>::MakeSptr(task);
    }
    auto listenerObject = listener->AsObject();
    if (listenerObject) {
        listenerObject->AddDeathRecipient(lifecycleListenerDeathRecipient_);
    }
    if (UnregisterSessionLifecycleListener(listener) != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener is invalid.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (bundleNameList.empty()) {
        listenersOfAllBundles_.emplace_back(listener);
        TLOGI(WmsLogTag::WMS_LIFE, "Register SessionLifecycleListener By All Bundles Finished.");
        return WMError::WM_OK;
    }
    for (const std::string& bundleName : bundleNameList) {
        if (bundleName.empty() || bundleName.size() > MAX_BUNDLE_NAME_LEN) {
            TLOGW(WmsLogTag::WMS_LIFE, "invalid bundleName");
            continue;
        }
        auto it = listenerMapByBundle_.find(bundleName);
        if (it != listenerMapByBundle_.end()) {
            if (it->second.size() >= MAX_LIFECYCLE_LISTENER_LIMIT) {
                TLOGE(WmsLogTag::WMS_LIFE, "The number of listeners has reached the upper limit.");
                return WMError::WM_ERROR_INVALID_OPERATION;
            }
            it->second.emplace_back(listener);
        } else {
            std::vector<sptr<ISessionLifecycleListener>> newListenerList;
            newListenerList.emplace_back(listener);
            listenerMapByBundle_.emplace(bundleName, newListenerList);
        }
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Register SessionLifecycleListener By Bundle Finished.");
    return WMError::WM_OK;
}

WMError SessionListenerController::UnregisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener)
{
    if (!listener) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener is invalid.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    const sptr<IRemoteObject> target = listener->AsObject();
    if (!target) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote listener is invalid.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    RemoveSessionLifecycleListener(target);
    TLOGI(WmsLogTag::WMS_LIFE, "Session Lifecycle Listener Unregister Finished.");
    return WMError::WM_OK;
}

void SessionListenerController::RemoveSessionLifecycleListener(const sptr<IRemoteObject>& target)
{
    auto compareByAsObject = [&target](const sptr<ISessionLifecycleListener>& item) {
        return (item != nullptr) && (item->AsObject() == target);
    };

    for (auto it = listenerMapById_.begin(); it != listenerMapById_.end();) {
        auto& listeners = it->second;
        listeners.erase(std::remove_if(listeners.begin(), listeners.end(), compareByAsObject), listeners.end());
        if (listeners.empty()) {
            it = listenerMapById_.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = listenerMapByBundle_.begin(); it != listenerMapByBundle_.end();) {
        auto& listeners = it->second;
        listeners.erase(std::remove_if(listeners.begin(), listeners.end(), compareByAsObject), listeners.end());
        if (listeners.empty()) {
            it = listenerMapByBundle_.erase(it);
        } else {
            ++it;
        }
    }

    auto iter = std::remove_if(listenersOfAllBundles_.begin(), listenersOfAllBundles_.end(), compareByAsObject);
    listenersOfAllBundles_.erase(iter, listenersOfAllBundles_.end());
    TLOGI(WmsLogTag::WMS_LIFE, "Remove listeners from map success.");
}

void SessionListenerController::NotifySessionLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent event,
    const SessionInfo& sessionInfo, LifeCycleChangeReason reason)
{
    std::string bundleName = sessionInfo.bundleName_;
    int32_t persistentId = sessionInfo.persistentId_;
    if (persistentId <= INVALID_SESSION_ID) {
        TLOGE(WmsLogTag::WMS_LIFE, "invalid persistentId!");
        return;
    }
    ISessionLifecycleListener::LifecycleEventPayload payload;
    ConstructPayload(payload, sessionInfo, 0, 0, 0, reason);
    NotifyMissionEvent(event, persistentId);
    taskScheduler_->PostAsyncTask(
        [weakThis = weak_from_this(), event, payload, bundleName, persistentId, where = __func__] {
            auto controller = weakThis.lock();
            if (controller == nullptr) {
                TLOGNE(WmsLogTag::WMS_LIFE, "controller is null.");
                return;
            }
            TLOGI(WmsLogTag::WMS_LIFE, "start notify listeners, bundleName:%{public}s, Id:%{public}d, event:%{public}d"
                ", reason: %{public}u",
                bundleName.c_str(), persistentId, event, payload.lifeCycleChangeReason_);
            controller->NotifyListeners(controller->listenerMapById_, persistentId, event, payload);
            controller->NotifyListeners(controller->listenerMapByBundle_, bundleName, event, payload);
            for (const auto& listener : controller->listenersOfAllBundles_) {
                if (listener != nullptr) {
                    listener->OnLifecycleEvent(event, payload);
                }
            }
        }, __func__);
}

template <typename KeyType, typename MapType>
void SessionListenerController::NotifyListeners(const MapType& listenerMap, const KeyType& key,
    const ISessionLifecycleListener::SessionLifecycleEvent event,
    const ISessionLifecycleListener::LifecycleEventPayload& payload)
{
    auto it = listenerMap.find(key);
    if (it != listenerMap.end()) {
        const auto& listeners = it->second;
        for (const auto& listener : listeners) {
            if (listener != nullptr) {
                listener->OnLifecycleEvent(event, payload);
            }
        }
    }
}

void SessionListenerController::NotifyMissionEvent(
    ISessionLifecycleListener::SessionLifecycleEvent event, int32_t persistentId)
{
    switch (event) {
        case ISessionLifecycleListener::SessionLifecycleEvent::CREATED:
            NotifySessionCreated(persistentId);
            break;
        case ISessionLifecycleListener::SessionLifecycleEvent::DESTROYED:
            NotifySessionDestroyed(persistentId);
            break;
        case ISessionLifecycleListener::SessionLifecycleEvent::BACKGROUND:
            NotifySessionBackground(persistentId);
            break;
        default:
            break;
    }
}

void SessionListenerController::NotifySessionTransferToTargetScreenEvent(const SessionInfo& sessionInfo,
    const uint32_t resultCode, const uint64_t fromScreenId, const uint64_t toScreenId,
    LifeCycleChangeReason reason)
{
    int32_t persistentId = sessionInfo.persistentId_;
    if (persistentId <= INVALID_SESSION_ID) {
        TLOGE(WmsLogTag::WMS_LIFE, "invalid persistentId!");
        return;
    }
    std::string bundleName = sessionInfo.bundleName_;
    auto event = ISessionLifecycleListener::SessionLifecycleEvent::TRANSFER_TO_TARGET_SCREEN;
    ISessionLifecycleListener::LifecycleEventPayload payload;
    ConstructPayload(payload, sessionInfo, resultCode, fromScreenId, toScreenId, reason);
    taskScheduler_->PostAsyncTask(
        [weakThis = weak_from_this(), event, payload, bundleName, persistentId, where = __func__] {
            auto controller = weakThis.lock();
            if (controller == nullptr) {
                TLOGNE(WmsLogTag::WMS_LIFE, "controller is null.");
                return;
            }
            TLOGNI(WmsLogTag::WMS_LIFE,
                "start notify listeners, bundleName:%{public}s, persistentId:%{public}d, event:%{public}d, "
                "reason: %{public}u",
                bundleName.c_str(), persistentId, event, payload.lifeCycleChangeReason_);
            controller->NotifyListeners(controller->listenerMapById_, persistentId, event, payload);
            controller->NotifyListeners(controller->listenerMapByBundle_, bundleName, event, payload);
            for (const auto& listener : controller->listenersOfAllBundles_) {
                if (listener != nullptr) {
                    listener->OnLifecycleEvent(event, payload);
                }
            }
        }, __func__);
}

bool SessionListenerController::IsListenerMapByIdSizeReachLimit() const
{
    return taskScheduler_->PostSyncTask([this] {
        return listenerMapById_.size() >= MAX_LISTEN_TARGET_LIMIT;
    }, __func__);
}

bool SessionListenerController::IsListenerMapByBundleSizeReachLimit(bool isBundleNameListEmpty) const
{
    return taskScheduler_->PostSyncTask([this, isBundleNameListEmpty] {
        if (isBundleNameListEmpty) {
            return listenersOfAllBundles_.size() >= MAX_LIFECYCLE_LISTENER_LIMIT;
        }
        return listenerMapByBundle_.size() >= MAX_LISTEN_TARGET_LIMIT;
    }, __func__);
}
} // namespace Rosen
} // namespace OHOS
