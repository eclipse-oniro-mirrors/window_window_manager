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

#include "session_manager_lite.h"
#include <ipc_skeleton.h>
#include <iremote_stub.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "scene_session_manager_lite_proxy.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_DISPLAY, "SessionManagerLite" };
}
std::unordered_map<int32_t, sptr<SessionManagerLite>> SessionManagerLite::sessionManagerLiteMap_ = {};
std::mutex SessionManagerLite::sessionManagerLiteMapMutex_;

SessionManagerServiceLiteRecoverListener::SessionManagerServiceLiteRecoverListener(int32_t userId) : userId_(userId) {}

int32_t SessionManagerServiceLiteRecoverListener::OnRemoteRequest(uint32_t code,
                                                                  MessageParcel& data,
                                                                  MessageParcel& reply,
                                                                  MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TLOGE(WmsLogTag::WMS_RECOVER, "InterfaceToken check failed");
        return ERR_TRANSACTION_FAILED;
    }
    auto msgId = static_cast<SessionManagerServiceRecoverMessage>(code);
    switch (msgId) {
        case SessionManagerServiceRecoverMessage::TRANS_ID_ON_SESSION_MANAGER_SERVICE_RECOVER: {
            auto sessionManagerService = data.ReadRemoteObject();
            // Even if sessionManagerService is null, the recovery process is still required.
            OnSessionManagerServiceRecover(sessionManagerService);
            break;
        }
        case SessionManagerServiceRecoverMessage::TRANS_ID_ON_WMS_CONNECTION_CHANGED: {
            int32_t wmsUserId = INVALID_USER_ID;
            int32_t screenId = DEFAULT_SCREEN_ID;
            bool isConnected = false;
            if (!data.ReadInt32(wmsUserId) || !data.ReadInt32(screenId) || !data.ReadBool(isConnected)) {
                TLOGE(WmsLogTag::WMS_MULTI_USER, "Read data failed in lite!");
                return ERR_TRANSACTION_FAILED;
            }
            if (isConnected) {
                // Even if data.ReadRemoteObject() is null, the WMS connection still needs to be notified.
                OnWMSConnectionChanged(wmsUserId, screenId, isConnected, data.ReadRemoteObject());
            } else {
                OnWMSConnectionChanged(wmsUserId, screenId, isConnected, nullptr);
            }
            break;
        }
        default:
            TLOGW(WmsLogTag::WMS_RECOVER, "unknown transaction code %{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_NONE;
}

void SessionManagerServiceLiteRecoverListener::OnSessionManagerServiceRecover(
    const sptr<IRemoteObject>& sessionManagerService)
{
    SessionManagerLite::GetInstance(userId_).Clear();
    SessionManagerLite::GetInstance(userId_).ClearSessionManagerProxy();

    auto sms = iface_cast<ISessionManagerService>(sessionManagerService);
    SessionManagerLite::GetInstance(userId_).RecoverSessionManagerService(sms);
}

void SessionManagerServiceLiteRecoverListener::OnWMSConnectionChanged(int32_t wmsUserId,
                                                                      int32_t screenId,
                                                                      bool isConnected,
                                                                      const sptr<IRemoteObject>& sessionManagerService)
{
    auto sms = iface_cast<ISessionManagerService>(sessionManagerService);
    SessionManagerLite::GetInstance(userId_).OnWMSConnectionChanged(wmsUserId, screenId, isConnected, sms);
}

class SceneSessionManagerLiteProxyMock : public SceneSessionManagerLiteProxy {
public:
    explicit SceneSessionManagerLiteProxyMock(const sptr<IRemoteObject>& impl)
        : SceneSessionManagerLiteProxy(impl) {}
    virtual ~SceneSessionManagerLiteProxyMock() = default;

    WSError RegisterSessionListener(const sptr<ISessionListener>& listener, bool isRecover = false) override
    {
        TLOGI(WmsLogTag::DEFAULT, "called");
        auto ret = SceneSessionManagerLiteProxy::RegisterSessionListener(listener);
        if (ret != WSError::WS_OK) {
            return ret;
        }
        if (isRecover) {
            TLOGI(WmsLogTag::DEFAULT, "Recover mode, no need to save listener");
            return WSError::WS_OK;
        }
        SessionManagerLite::GetInstance().SaveSessionListener(listener);
        return WSError::WS_OK;
    }
    WSError UnRegisterSessionListener(const sptr<ISessionListener>& listener) override
    {
        TLOGI(WmsLogTag::DEFAULT, "called");
        auto ret = SceneSessionManagerLiteProxy::UnRegisterSessionListener(listener);
        SessionManagerLite::GetInstance().DeleteSessionListener(listener);
        return ret;
    }

private:
    static inline BrokerDelegator<SceneSessionManagerLiteProxyMock> delegator_;
};

SessionManagerLite& SessionManagerLite::GetInstance()
{
    static auto instance = sptr<SessionManagerLite>::MakeSptr();
    return *instance;
}

SessionManagerLite& SessionManagerLite::GetInstance(const int32_t userId)
{
    if (userId <= INVALID_USER_ID) {
        return GetInstance();
    }
    //multi-instance mode
    std::lock_guard<std::mutex> lock(sessionManagerLiteMapMutex_);
    auto iter = sessionManagerLiteMap_.find(userId);
    if (iter != sessionManagerLiteMap_.end() && iter->second) {
        return *iter->second;
    }
    auto instance = sptr<SessionManagerLite>::MakeSptr(userId);
    sessionManagerLiteMap_.insert({userId, instance});
    TLOGI(WmsLogTag::WMS_MULTI_USER, "get new instance, userId: %{public}d", userId);
    return *sessionManagerLiteMap_[userId];
}

SessionManagerLite::SessionManagerLite(const int32_t userId) : userId_(userId) {}

SessionManagerLite::~SessionManagerLite()
{
    sptr<IRemoteObject> remoteObject = nullptr;
    if (mockSessionManagerServiceProxy_) {
        remoteObject = mockSessionManagerServiceProxy_->AsObject();
    }
    if (remoteObject) {
        remoteObject->RemoveDeathRecipient(foundationDeath_);
    }
    TLOGI(WmsLogTag::WMS_SCB, "destroyed, userId: %{public}d", userId_);
}

void SessionManagerLite::ClearSessionManagerProxy()
{
    WLOGFI("enter");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    sessionManagerServiceProxy_ = nullptr;
    sceneSessionManagerLiteProxy_ = nullptr;
    screenSessionManagerLiteProxy_ = nullptr;
}

sptr<ISceneSessionManagerLite> SessionManagerLite::GetSceneSessionManagerLiteProxy()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    InitSessionManagerServiceProxy();
    InitSceneSessionManagerLiteProxy();
    return sceneSessionManagerLiteProxy_;
}

sptr<IScreenSessionManagerLite> SessionManagerLite::GetScreenSessionManagerLiteProxy()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    InitSessionManagerServiceProxy();
    InitScreenSessionManagerLiteProxy();
    return screenSessionManagerLiteProxy_;
}

sptr<ISessionManagerService> SessionManagerLite::GetSessionManagerServiceProxy()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    InitSessionManagerServiceProxy();
    InitSceneSessionManagerLiteProxy();
    return sessionManagerServiceProxy_;
}

void SessionManagerLite::SaveSessionListener(const sptr<ISessionListener>& listener)
{
    if (listener == nullptr) {
        TLOGW(WmsLogTag::DEFAULT, "listener is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> guard(listenerLock_);
    auto it = std::find_if(sessionListeners_.begin(), sessionListeners_.end(),
        [&listener](const sptr<ISessionListener>& item) {
            return (item && item->AsObject() == listener->AsObject());
        });
    if (it != sessionListeners_.end()) {
        TLOGW(WmsLogTag::DEFAULT, "listener was already added, do not add again");
        return;
    }
    sessionListeners_.emplace_back(listener);
}

void SessionManagerLite::DeleteSessionListener(const sptr<ISessionListener>& listener)
{
    TLOGI(WmsLogTag::DEFAULT, "called");
    std::lock_guard<std::recursive_mutex> guard(listenerLock_);
    auto it = std::find_if(sessionListeners_.begin(), sessionListeners_.end(),
        [&listener](const sptr<ISessionListener>& item) {
            return (item && item->AsObject() == listener->AsObject());
        });
    if (it != sessionListeners_.end()) {
        sessionListeners_.erase(it);
    }
}

void SessionManagerLite::DeleteAllSessionListeners()
{
    std::lock_guard<std::recursive_mutex> guard(listenerLock_);
    sessionListeners_.clear();
}

void SessionManagerLite::RecoverSessionManagerService(const sptr<ISessionManagerService>& sessionManagerService)
{
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        sessionManagerServiceProxy_ = sessionManagerService;
    }
    GetSceneSessionManagerLiteProxy();
    ReregisterSessionListener();
    UserSwitchCallbackFunc callbackFunc = nullptr;
    {
        std::lock_guard<std::mutex> lock(userSwitchCallbackFuncMutex_);
        callbackFunc = userSwitchCallbackFunc_;
    }
    if (callbackFunc) {
        TLOGI(WmsLogTag::WMS_RECOVER, "begin user switch");
        callbackFunc();
    } else {
        TLOGE(WmsLogTag::WMS_RECOVER, "callback func is null");
    }
}

void SessionManagerLite::ReregisterSessionListener()
{
    sptr<ISceneSessionManagerLite> sceneSessionManagerLiteProxy = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        sceneSessionManagerLiteProxy = sceneSessionManagerLiteProxy_;
    }
    if (sceneSessionManagerLiteProxy == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "sceneSessionManagerLiteProxy is null");
        return;
    }

    std::lock_guard<std::recursive_mutex> guard(listenerLock_);
    TLOGI(WmsLogTag::WMS_RECOVER, "listener count=%{public}" PRIu64,
        static_cast<int64_t>(sessionListeners_.size()));
    for (const auto& listener : sessionListeners_) {
        auto ret = sceneSessionManagerLiteProxy->RegisterSessionListener(listener, true);
        if (ret != WSError::WS_OK) {
            TLOGW(WmsLogTag::WMS_RECOVER, "failed, ret=%{public}" PRId32, ret);
        }
    }
}

void SessionManagerLite::RegisterUserSwitchListener(const UserSwitchCallbackFunc& callbackFunc)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "enter");
    std::lock_guard<std::mutex> lock(userSwitchCallbackFuncMutex_);
    userSwitchCallbackFunc_ = callbackFunc;
}

void SessionManagerLite::OnWMSConnectionChanged(
    int32_t userId, int32_t screenId, bool isConnected, const sptr<ISessionManagerService>& sessionManagerService)
{
    int32_t lastUserId = INVALID_USER_ID;
    int32_t lastScreenId = DEFAULT_SCREEN_ID;
    {
        // The mutex ensures the timing of the following variable states in multiple threads
        std::lock_guard<std::mutex> lock(wmsConnectionMutex_);
        lastUserId = currentWMSUserId_;
        lastScreenId = currentScreenId_;
        if (isConnected) {
            currentWMSUserId_ = userId;
            currentScreenId_ = screenId;
        }
        // isWMSConnected_ only represents the wms connection status of the active user
        if (currentWMSUserId_ == userId) {
            isWMSConnected_ = isConnected;
        }
    }
    TLOGD(WmsLogTag::WMS_MULTI_USER,
          "Lite: curUserId=%{public}d, oldUserId=%{public}d, screenId=%{public}d, isConnected=%{public}d",
          userId, lastUserId, screenId, isConnected);
    if (isConnected && lastUserId > INVALID_USER_ID && lastUserId != userId) {
        // Notify the user that the old wms has been disconnected.
        OnWMSConnectionChangedCallback(lastUserId, lastScreenId, false);
        OnUserSwitch(sessionManagerService);
    }
    // Notify the user that the current wms connection has changed.
    OnWMSConnectionChangedCallback(userId, screenId, isConnected);
}

void SessionManagerLite::OnUserSwitch(const sptr<ISessionManagerService>& sessionManagerService)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "User switched Lite");
    {
        Clear();
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        sessionManagerServiceProxy_ = sessionManagerService;
        sceneSessionManagerLiteProxy_ = nullptr;
        InitSceneSessionManagerLiteProxy();
        if (!sceneSessionManagerLiteProxy_) {
            TLOGE(WmsLogTag::WMS_MULTI_USER, "sceneSessionManagerLiteProxy is null");
            return;
        }
    }
    ReregisterSessionListener();
    UserSwitchCallbackFunc callbackFunc = nullptr;
    {
        std::lock_guard<std::mutex> lock(userSwitchCallbackFuncMutex_);
        callbackFunc = userSwitchCallbackFunc_;
    }
    if (callbackFunc) {
        TLOGI(WmsLogTag::WMS_MULTI_USER, "run callback func");
        callbackFunc();
    } else {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "callback func is null");
    }
}

void SessionManagerLite::InitSessionManagerServiceProxy()
{
    auto ret = WMError::WM_OK;
    sptr<IRemoteObject> remoteObject = nullptr;
    if (sessionManagerServiceProxy_) {
        return;
    }
    ret = InitMockSMSProxy();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SCB, "init mock failed");
        return;
    }
    RegisterSMSRecoverListener();
    ret = static_cast<WMError>(
        userId_ == INVALID_USER_ID
            ? mockSessionManagerServiceProxy_->GetSessionManagerService(remoteObject)
            : mockSessionManagerServiceProxy_->GetSessionManagerServiceByUserId(userId_, remoteObject));
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SCB, "get service failed, userId: %{public}d", userId_);
        return;
    }
    sessionManagerServiceProxy_ = iface_cast<ISessionManagerService>(remoteObject);
    if (!sessionManagerServiceProxy_) {
        TLOGE(WmsLogTag::WMS_SCB, "init proxy failed");
    }
}

void SessionManagerLite::InitScreenSessionManagerLiteProxy()
{
    if (screenSessionManagerLiteProxy_) {
        return;
    }
    if (!mockSessionManagerServiceProxy_) {
        WLOGFE("mockSessionManagerServiceProxy is nullptr");
        return;
    }

    sptr<IRemoteObject> remoteObject = nullptr;
    mockSessionManagerServiceProxy_->GetScreenSessionManagerLite(remoteObject);
    if (!remoteObject) {
        WLOGFW("Get screen session manager lite proxy failed, null");
        return;
    }
    screenSessionManagerLiteProxy_ = iface_cast<IScreenSessionManagerLite>(remoteObject);
    if (!screenSessionManagerLiteProxy_) {
        WLOGFW("Get screen session manager lite proxy failed, iface_cast null");
    }
}

void SessionManagerLite::InitSceneSessionManagerLiteProxy()
{
    if (sceneSessionManagerLiteProxy_) {
        return;
    }
    if (!sessionManagerServiceProxy_) {
        WLOGFE("sessionManagerServiceProxy is nullptr");
        return;
    }

    sptr<IRemoteObject> remoteObject = sessionManagerServiceProxy_->GetSceneSessionManagerLite();
    if (!remoteObject) {
        WLOGFW("Get scene session manager proxy failed, null");
        return;
    }
    sceneSessionManagerLiteProxy_ = iface_cast<ISceneSessionManagerLite>(remoteObject);
    if (sceneSessionManagerLiteProxy_) {
        ssmDeath_ = sptr<SSMDeathRecipientLite>::MakeSptr(userId_);
        if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(ssmDeath_)) {
            WLOGFE("Failed to add death recipient");
            return;
        }
    } else {
        WLOGFW("Get scene session manager proxy failed, iface_cast null");
    }
}

void SessionManagerLite::Clear()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if ((sceneSessionManagerLiteProxy_ != nullptr) && (sceneSessionManagerLiteProxy_->AsObject() != nullptr)) {
        sceneSessionManagerLiteProxy_->AsObject()->RemoveDeathRecipient(ssmDeath_);
    }
}

SSMDeathRecipientLite::SSMDeathRecipientLite(const int32_t userId) : userId_(userId) {}

void SSMDeathRecipientLite::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    TLOGI(WmsLogTag::WMS_SCB, "ssm lite OnRemoteDied");
    SessionManagerLite::GetInstance(userId_).Clear();
    SessionManagerLite::GetInstance(userId_).ClearSessionManagerProxy();
}

WMError SessionManagerLite::RegisterWMSConnectionChangedListener(const WMSConnectionChangedCallbackFunc& callbackFunc)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "Lite in");
    if (callbackFunc == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Lite callbackFunc is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    bool isWMSAlreadyConnected = false;
    int32_t userId = INVALID_USER_ID;
    int32_t screenId = DEFAULT_SCREEN_ID;
    {
        // The mutex ensures the timing of the following variable states in multiple threads
        std::lock_guard<std::mutex> lock(wmsConnectionMutex_);
        wmsConnectionChangedFunc_ = callbackFunc;
        isWMSAlreadyConnected = isWMSConnected_ && (currentWMSUserId_ > INVALID_USER_ID);
        userId = currentWMSUserId_;
        screenId = currentScreenId_;
    }
    if (isWMSAlreadyConnected) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "Lite WMS already connected, notify immediately");
        OnWMSConnectionChangedCallback(userId, screenId, true);
    }
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto ret = InitMockSMSProxy();
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_MULTI_USER, "Init mock session manager service failed");
            return ret;
        }
    }
    RegisterSMSRecoverListener();
    return WMError::WM_OK;
}

WMError SessionManagerLite::UnregisterWMSConnectionChangedListener()
{
    {
        std::lock_guard<std::mutex> lock(wmsConnectionMutex_);
        wmsConnectionChangedFunc_ = nullptr;
    }
    UnregisterSMSRecoverListener();
    return WMError::WM_OK;
}

WMError SessionManagerLite::InitMockSMSProxy()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Failed to get system ability mgr.");
        return WMError::WM_ERROR_NULLPTR;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(WINDOW_MANAGER_SERVICE_ID);
    if (!remoteObject) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Remote object is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    mockSessionManagerServiceProxy_ = iface_cast<IMockSessionManagerInterface>(remoteObject);
    if (!mockSessionManagerServiceProxy_) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Get mock session manager service proxy failed, nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }

    if (GetUserIdByUid(getuid()) != SYSTEM_USERID || isFoundationListenerRegistered_) {
        return WMError::WM_OK;
    }
    if (!foundationDeath_) {
        foundationDeath_ = sptr<FoundationDeathRecipientLite>::MakeSptr(userId_);
    }
    if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(foundationDeath_)) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Failed to add death recipient");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    isFoundationListenerRegistered_ = true;
    return WMError::WM_OK;
}

void SessionManagerLite::RegisterSMSRecoverListener()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!recoverListenerRegistered_) {
        if (!mockSessionManagerServiceProxy_) {
            TLOGE(WmsLogTag::WMS_RECOVER, "mockSessionManagerServiceProxy is null");
            return;
        }
        TLOGD(WmsLogTag::WMS_RECOVER, "Register recover listener, userId_: %{public}d", userId_);
        smsRecoverListener_ = sptr<SessionManagerServiceLiteRecoverListener>::MakeSptr(userId_);
        std::string identity = IPCSkeleton::ResetCallingIdentity();
        mockSessionManagerServiceProxy_->RegisterSMSRecoverListener(userId_, true, smsRecoverListener_);
        IPCSkeleton::SetCallingIdentity(identity);
        recoverListenerRegistered_ = true;
    }
}

void SessionManagerLite::UnregisterSMSRecoverListener()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    recoverListenerRegistered_ = false;
    if (mockSessionManagerServiceProxy_) {
        TLOGD(WmsLogTag::WMS_RECOVER, "UnRegister recover listener, userId_: %{public}d", userId_);
        mockSessionManagerServiceProxy_->UnregisterSMSRecoverListener(userId_, true);
    }
}

void SessionManagerLite::OnWMSConnectionChangedCallback(int32_t userId, int32_t screenId, bool isConnected)
{
    WMSConnectionChangedCallbackFunc callbackFunc = nullptr;
    {
        std::lock_guard<std::mutex> lock(wmsConnectionMutex_);
        callbackFunc = wmsConnectionChangedFunc_;
    }
    if (callbackFunc) {
        TLOGI(WmsLogTag::WMS_MULTI_USER,
            "WMS connection changed with userId=%{public}d, screenId=%{public}d, isConnected=%{public}d", userId,
            screenId, isConnected);
        callbackFunc(userId, screenId, isConnected);
    } else {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "Lite WMS CallbackFunc is null.");
    }
}

FoundationDeathRecipientLite::FoundationDeathRecipientLite(const int32_t userId) : userId_(userId) {}

void FoundationDeathRecipientLite::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "Foundation died");
    SessionManagerLite::GetInstance(userId_).OnFoundationDied();
}

void SessionManagerLite::OnFoundationDied()
{
    TLOGI(WmsLogTag::WMS_RECOVER, "enter");
    {
        std::lock_guard<std::mutex> lock(wmsConnectionMutex_);
        isWMSConnected_ = false;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    isFoundationListenerRegistered_ = false;
    recoverListenerRegistered_ = false;
    mockSessionManagerServiceProxy_ = nullptr;
    sessionManagerServiceProxy_ = nullptr;
    sceneSessionManagerLiteProxy_ = nullptr;
}
} // namespace OHOS::Rosen
