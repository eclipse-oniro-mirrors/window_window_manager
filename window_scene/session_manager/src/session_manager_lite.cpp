/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
    SessionManagerLite::GetInstance(userId_).RemoveSSMDeathRecipient();
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
    // Note: iface_cast<ISceneSessionManagerLite> will use it, and sceneSessionManagerLiteProxy_
    // is the instance of the SceneSessionManagerLiteProxyMock.
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
        remoteObject->RemoveDeathRecipient(mockFoundationDeathRecipient_);
        TLOGD(WmsLogTag::DEFAULT, "Remove death recipient success");
    }
    TLOGI(WmsLogTag::DEFAULT, "Destroyed, userId=%{public}d", userId_);
}

void SessionManagerLite::ClearSessionManagerProxy()
{
    {
        std::lock_guard<std::mutex> lock(smsProxyMutex_);
        sessionManagerServiceProxy_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(ssmLiteProxyMutex_);
        sceneSessionManagerLiteProxy_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(screenSMLiteProxyMutex_);
        screenSessionManagerLiteProxy_ = nullptr;
    }
    TLOGI(WmsLogTag::DEFAULT, "Proxy clear success");
}

sptr<ISceneSessionManagerLite> SessionManagerLite::GetSceneSessionManagerLiteProxy()
{
    WMError ret = InitSessionManagerServiceProxy();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "Init session manager service proxy failed");
        return nullptr;
    }
    ret = InitSceneSessionManagerLiteProxy();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "Init scene session manager lite proxy failed");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(ssmLiteProxyMutex_);
    return sceneSessionManagerLiteProxy_;
}

sptr<IScreenSessionManagerLite> SessionManagerLite::GetScreenSessionManagerLiteProxy()
{
    WMError ret = InitSessionManagerServiceProxy();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "Init session manager service proxy failed");
        return nullptr;
    }
    ret = InitScreenSessionManagerLiteProxy();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "Init screen session manager lite proxy failed");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(screenSMLiteProxyMutex_);
    return screenSessionManagerLiteProxy_;
}

sptr<ISessionManagerService> SessionManagerLite::GetSessionManagerServiceProxy()
{
    WMError ret = InitSessionManagerServiceProxy();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "Init session manager service proxy failed");
        return nullptr;
    }
    InitSceneSessionManagerLiteProxy();
    std::lock_guard<std::mutex> lock(smsProxyMutex_);
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
        std::lock_guard<std::mutex> lock(smsProxyMutex_);
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
        std::lock_guard<std::mutex> lock(ssmLiteProxyMutex_);
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
    TLOGD(WmsLogTag::WMS_MULTI_USER, "enter");
    RemoveSSMDeathRecipient();
    {
        std::lock_guard<std::mutex> lock(smsProxyMutex_);
        sessionManagerServiceProxy_ = sessionManagerService;
    }
    {
        std::lock_guard<std::mutex> lock(ssmLiteProxyMutex_);
        sceneSessionManagerLiteProxy_ = nullptr;
    }
    auto ret = InitSceneSessionManagerLiteProxy();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Init scene session manager lite proxy failed");
        return;
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

WMError SessionManagerLite::InitSessionManagerServiceProxy()
{
    {
        std::lock_guard<std::mutex> lock(smsProxyMutex_);
        if (sessionManagerServiceProxy_) {
            return WMError::WM_OK;
        }
    }
    // Begin init.
    WMError ret = RegisterSMSRecoverListener();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_RECOVER, "Register recover listener failed");
        return ret;
    }
    auto mockSMSProxy = GetMockSessionManagerServiceProxy();
    if (!mockSMSProxy) {
        TLOGE(WmsLogTag::DEFAULT, "Get mock session manager service proxy failed");
        return WMError::WM_ERROR_NULLPTR;
    }
    sptr<IRemoteObject> remoteObject = nullptr;
    ret = static_cast<WMError>(
        userId_ == INVALID_USER_ID
            ? mockSMSProxy->GetSessionManagerService(remoteObject)
            : mockSMSProxy->GetSessionManagerServiceByUserId(userId_, remoteObject));
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "Get session manager service remote failed, userId=%{public}d", userId_);
        return ret;
    }
    std::lock_guard<std::mutex> lock(smsProxyMutex_);
    sessionManagerServiceProxy_ = iface_cast<ISessionManagerService>(remoteObject);
    if (!sessionManagerServiceProxy_) {
        TLOGE(WmsLogTag::DEFAULT, "IPC convert failed");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::DEFAULT, "Init success");
    return WMError::WM_OK;
}

WMError SessionManagerLite::InitScreenSessionManagerLiteProxy()
{
    {
        std::lock_guard<std::mutex> lock(screenSMLiteProxyMutex_);
        if (screenSessionManagerLiteProxy_) {
            return WMError::WM_OK;
        }
    }
    auto mockSMSProxy = GetMockSessionManagerServiceProxy();
    if (!mockSMSProxy) {
        TLOGE(WmsLogTag::DEFAULT, "Get mock session manager service proxy failed");
        return WMError::WM_ERROR_NULLPTR;
    }
    sptr<IRemoteObject> remoteObject = nullptr;
    mockSMSProxy->GetScreenSessionManagerLite(remoteObject);
    if (!remoteObject) {
        TLOGE(WmsLogTag::DEFAULT, "Get remote object failed");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::mutex> lock(screenSMLiteProxyMutex_);
    screenSessionManagerLiteProxy_ = iface_cast<IScreenSessionManagerLite>(remoteObject);
    if (!screenSessionManagerLiteProxy_) {
        TLOGE(WmsLogTag::DEFAULT, "IPC convert failed");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::DEFAULT, "Init success");
    return WMError::WM_OK;
}

WMError SessionManagerLite::InitSceneSessionManagerLiteProxy()
{
    {
        std::lock_guard<std::mutex> lock(ssmLiteProxyMutex_);
        if (sceneSessionManagerLiteProxy_) {
            return WMError::WM_OK;
        }
    }
    // Begin init.
    sptr<IRemoteObject> remoteObject = nullptr;
    {
        std::lock_guard<std::mutex> lock(smsProxyMutex_);
        if (!sessionManagerServiceProxy_) {
            TLOGE(WmsLogTag::DEFAULT, "Session manager proxy is null");
            return WMError::WM_ERROR_NULLPTR;
        }
        remoteObject = sessionManagerServiceProxy_->GetSceneSessionManagerLite();
    }
    if (!remoteObject) {
        TLOGE(WmsLogTag::DEFAULT, "Get remote object failed");
        return WMError::WM_ERROR_NULLPTR;
    }
    // Covert to get proxy and add death recipient.
    sptr<SSMDeathRecipientLite> deathRecipient = nullptr;
    {
        std::lock_guard<std::mutex> lock(ssmLiteProxyMutex_);
        sceneSessionManagerLiteProxy_ = iface_cast<ISceneSessionManagerLite>(remoteObject);
        if (!sceneSessionManagerLiteProxy_) {
            TLOGE(WmsLogTag::DEFAULT, "IPC convert failed");
            return WMError::WM_ERROR_NULLPTR;
        }
        if (!ssmLiteDeathRecipient_) {
            ssmLiteDeathRecipient_ = sptr<SSMDeathRecipientLite>::MakeSptr(userId_);
        }
        deathRecipient = ssmLiteDeathRecipient_;
    }
    if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(deathRecipient)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to add death recipient");
        std::lock_guard<std::mutex> lock(ssmLiteProxyMutex_);
        sceneSessionManagerLiteProxy_ = nullptr;
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::DEFAULT, "Init success");
    return WMError::WM_OK;
}

void SessionManagerLite::RemoveSSMDeathRecipient()
{
    sptr<IRemoteObject> remoteObject = nullptr;
    sptr<SSMDeathRecipientLite> deathRecipient = nullptr;
    {
        std::lock_guard<std::mutex> lock(ssmLiteProxyMutex_);
        if (sceneSessionManagerLiteProxy_) {
            remoteObject = sceneSessionManagerLiteProxy_->AsObject();
        }
        deathRecipient = ssmLiteDeathRecipient_;
    }
    if (remoteObject) {
        remoteObject->RemoveDeathRecipient(deathRecipient);
        TLOGI(WmsLogTag::DEFAULT, "Removed success");
    }
    TLOGD(WmsLogTag::DEFAULT, "end");
}

SSMDeathRecipientLite::SSMDeathRecipientLite(const int32_t userId) : userId_(userId) {}

void SSMDeathRecipientLite::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    TLOGI(WmsLogTag::DEFAULT, "Scene session manager service died");
    SessionManagerLite::GetInstance(userId_).RemoveSSMDeathRecipient();
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
    WMError ret = RegisterSMSRecoverListener();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Register sms recover listener failed");
    }
    return ret;
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

sptr<IMockSessionManagerInterface> SessionManagerLite::GetMockSessionManagerServiceProxy()
{
    {
        std::lock_guard<std::mutex> lock(mockSMSProxyMutex_);
        if (mockSessionManagerServiceProxy_) {
            return mockSessionManagerServiceProxy_;
        }
    }
    auto clientUserId = GetUserIdByUid(getuid());
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to get SA manager");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(WINDOW_MANAGER_SERVICE_ID);
    if (!remoteObject) {
        TLOGE(WmsLogTag::DEFAULT, "Get remote object failed");
        return nullptr;
    }
    auto proxy = iface_cast<IMockSessionManagerInterface>(remoteObject);
    if (!proxy) {
        TLOGE(WmsLogTag::DEFAULT, "IPC covert failed");
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(mockSMSProxyMutex_);
        mockSessionManagerServiceProxy_ = proxy;
        if (clientUserId != SYSTEM_USERID) {
            // Note: If not u0 app, no need to add foundation death recipient and just return.
            return mockSessionManagerServiceProxy_;
        }
    }
    // Begin to add foundation death recipient.
    sptr<FoundationDeathRecipientLite> deathRecipient = nullptr;
    {
        std::lock_guard<std::mutex> lock(mockSMSProxyMutex_);
        if (!mockFoundationDeathRecipient_) {
            mockFoundationDeathRecipient_ = sptr<FoundationDeathRecipientLite>::MakeSptr(userId_);
        }
        deathRecipient = mockFoundationDeathRecipient_;
    }
    if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(deathRecipient)) {
        TLOGE(WmsLogTag::DEFAULT, "Add death recipient failed");
        {
            // Note: If add death recipient failed, mock sms proxy should be re-initialize.
            std::lock_guard<std::mutex> lock(mockSMSProxyMutex_);
            mockSessionManagerServiceProxy_ = nullptr;
        }
        return nullptr;
    }
    TLOGI(WmsLogTag::DEFAULT, "Init success");
    std::lock_guard<std::mutex> lock(mockSMSProxyMutex_);
    return mockSessionManagerServiceProxy_;
}

WMError SessionManagerLite::RegisterSMSRecoverListener()
{
    {
        std::lock_guard<std::mutex> lock(smsRecoverListenerMutex_);
        if (isRecoverListenerRegistered_) {
            return WMError::WM_OK;
        }
    }
    auto mockSMSProxy = GetMockSessionManagerServiceProxy();
    if (mockSMSProxy == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "Get mock session manager service proxy failed");
        return WMError::WM_ERROR_NULLPTR;
    }
    // Regsiter session manager service recover listener to foundation process.
    sptr<IRemoteObject> listener = nullptr;
    {
        std::lock_guard<std::mutex> lock(smsRecoverListenerMutex_);
        if (!smsRecoverListener_) {
            smsRecoverListener_ = sptr<SessionManagerServiceLiteRecoverListener>::MakeSptr(userId_);
        }
        listener = smsRecoverListener_;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    // Note: No need to judge the err code, because ipc not necessarily return ERR_OK even though ipc send success.
    mockSMSProxy->RegisterSMSRecoverListener(userId_, true, listener);
    IPCSkeleton::SetCallingIdentity(identity);
    {
        std::lock_guard<std::mutex> lock(smsRecoverListenerMutex_);
        isRecoverListenerRegistered_ = true;
    }
    return WMError::WM_OK;
}

void SessionManagerLite::UnregisterSMSRecoverListener()
{
    auto mockSMSProxy = GetMockSessionManagerServiceProxy();
    if (mockSMSProxy) {
        mockSMSProxy->UnregisterSMSRecoverListener(userId_, true);
    }
    std::lock_guard<std::mutex> lock(smsRecoverListenerMutex_);
    isRecoverListenerRegistered_ = false;
    TLOGI(WmsLogTag::WMS_RECOVER, "Unregister end, userId_=%{public}d", userId_);
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
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Callback is null");
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
    TLOGI(WmsLogTag::DEFAULT, "enter");
    {
        std::lock_guard<std::mutex> lock(wmsConnectionMutex_);
        isWMSConnected_ = false;
    }
    {
        std::lock_guard<std::mutex> lock(smsRecoverListenerMutex_);
        isRecoverListenerRegistered_ = false;
        smsRecoverListener_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(mockSMSProxyMutex_);
        mockSessionManagerServiceProxy_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(smsProxyMutex_);
        sessionManagerServiceProxy_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(ssmLiteProxyMutex_);
        sceneSessionManagerLiteProxy_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(screenSMLiteProxyMutex_);
        screenSessionManagerLiteProxy_ = nullptr;
    }
}
} // namespace OHOS::Rosen
