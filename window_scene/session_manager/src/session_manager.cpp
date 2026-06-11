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

#include "session_manager.h"
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>
#include "session_manager_lite.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
std::unordered_map<int32_t, sptr<SessionManager>> SessionManager::sessionManagerMap_ = {};
std::mutex SessionManager::sessionManagerMapMutex_;

SessionManagerServiceRecoverListener::SessionManagerServiceRecoverListener(int32_t userId) : userId_(userId) {}

int32_t SessionManagerServiceRecoverListener::OnRemoteRequest(uint32_t code,
                                                              MessageParcel& data,
                                                              MessageParcel& reply,
                                                              MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TLOGE(WmsLogTag::WMS_RECOVER, "InterfaceToken check failed, userId=%{public}d", userId_);
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
            int32_t pid = INVALID_PID;
            int32_t fromUserId = INVALID_USER_ID;
            int32_t fromPid = INVALID_PID;
            if (!data.ReadInt32(wmsUserId) || !data.ReadInt32(screenId) ||
                !data.ReadBool(isConnected) || !data.ReadInt32(pid) ||
                !data.ReadInt32(fromUserId) || !data.ReadInt32(fromPid)) {
                TLOGE(WmsLogTag::WMS_RECOVER, "Read data failed, userId=%{public}d", userId_);
                return ERR_TRANSACTION_FAILED;
            }
            if (isConnected) {
                OnWMSConnectionChanged(wmsUserId, screenId, isConnected, pid, fromUserId,
                    fromPid, data.ReadRemoteObject());
            } else {
                OnWMSConnectionChanged(wmsUserId, screenId, isConnected, pid, fromUserId, fromPid, nullptr);
            }
            break;
        }
        default:
            TLOGW(WmsLogTag::WMS_RECOVER, "unknown transaction code: %{public}d, userId=%{public}d", code, userId_);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_NONE;
}

void SessionManagerServiceRecoverListener::OnSessionManagerServiceRecover(
    const sptr<IRemoteObject>& sessionManagerService)
{
    TLOGD(WmsLogTag::WMS_RECOVER, "enter, userId=%{public}d", userId_);
    SessionManager::GetInstance(userId_).RemoveSSMDeathRecipient();
    SessionManager::GetInstance(userId_).ClearSessionManagerProxy();

    auto sms = iface_cast<ISessionManagerService>(sessionManagerService);
    SessionManager::GetInstance(userId_).RecoverSessionManagerService(sms);
}

void SessionManagerServiceRecoverListener::OnWMSConnectionChanged(
    int32_t wmsUserId, int32_t screenId, bool isConnected, int32_t pid,
    int32_t fromUserId, int32_t fromPid,
    const sptr<IRemoteObject>& sessionManagerService)
{
    TLOGD(WmsLogTag::WMS_RECOVER, "enter, userId=%{public}d", userId_);
    auto sms = iface_cast<ISessionManagerService>(sessionManagerService);
    SessionManager::GetInstance(userId_).OnWMSConnectionChanged(wmsUserId, screenId, isConnected,
        pid, fromUserId, fromPid, sms);
}

SessionManager::~SessionManager()
{
    UnregisterSMSRecoverListener();
    RemoveMockFoundationDeathRecipient();
    RemoveSSMDeathRecipient();
    TLOGI(WmsLogTag::WMS_SCB, "session manager destroyed, userId: %{public}d", userId_);
}

SessionManager::SessionManager(const int32_t userId) : userId_(userId) {}

SessionManager& SessionManager::GetInstance()
{
    static auto instance = sptr<SessionManager>::MakeSptr();
    return *instance;
}

SessionManager& SessionManager::GetInstance(const int32_t userId)
{
    if (userId <= INVALID_USER_ID) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "get default instance, userId: %{public}d", userId);
        return GetInstance();
    }
    std::lock_guard<std::mutex> lock(sessionManagerMapMutex_);
    auto iter = sessionManagerMap_.find(userId);
    if (iter != sessionManagerMap_.end() && iter->second) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "get existing instance, userId: %{public}d", userId);
        return *iter->second;
    }
    auto instance = sptr<SessionManager>::MakeSptr(userId);
    sessionManagerMap_.insert({ userId, instance });
    TLOGI(WmsLogTag::WMS_MULTI_USER, "get new instance, userId: %{public}d", userId);
    return *sessionManagerMap_[userId];
}

void SessionManager::OnWMSConnectionChangedCallback(int32_t userId, int32_t screenId, bool isConnected, int32_t pid)
{
    WMSConnectionChangedCallbackFunc callbackFunc = nullptr;
    {
        std::lock_guard<std::mutex> lock(wmsConnectionMutex_);
        if (!wmsConnectionChangedFunc_) {
            TLOGE(WmsLogTag::WMS_MULTI_USER,
                "callback func is null, inst=%{public}d", userId_);
            return;
        }
        callbackFunc = wmsConnectionChangedFunc_;
    }
    TLOGI(WmsLogTag::WMS_MULTI_USER,
        "WMS connection changed with userId=%{public}d, screenId=%{public}d, isConnected=%{public}d, pid=%{public}d",
        userId, screenId, isConnected, pid);
    callbackFunc(userId, screenId, isConnected, pid);
}
void SessionManager::OnWMSConnectionChanged(
    int32_t userId, int32_t screenId, bool isConnected,
    int32_t pid, int32_t fromUserId, int32_t fromPid,
    const sptr<ISessionManagerService>& sessionManagerService)
{
    TLOGI(WmsLogTag::WMS_MULTI_USER,
        "OnWMSConnectionChanged: inst=%{public}d, userId=%{public}d, screenId=%{public}d, "
        "isConnected=%{public}d, pid=%{public}d, fromUserId=%{public}d, fromPid=%{public}d",
        userId_, userId, screenId, isConnected, pid, fromUserId, fromPid);
    bool shouldUpdateCurrentState = false;
    {
        std::lock_guard<std::mutex> lock(wmsConnectionMutex_);
        if (userId_ == INVALID_USER_ID) {
            if (isConnected) {
                shouldUpdateCurrentState = screenId == currentServer_.screenId
                    || currentServer_.screenId == INVALID_SCREEN_ID_INT32;
            } else {
                shouldUpdateCurrentState = userId == currentServer_.userId;
            }
        } else {
            shouldUpdateCurrentState = userId == userId_;
        }
        if (isConnected && shouldUpdateCurrentState) {
            currentServer_ = { userId, screenId, pid };
        }
        if (shouldUpdateCurrentState) {
            isWMSConnected_ = isConnected;
        }
    }
    bool isUserSwitched = fromUserId != INVALID_USER_ID;
    TLOGI(WmsLogTag::WMS_MULTI_USER,
        "State updated: inst=%{public}d, shouldUpdate=%{public}d, isUserSwitched=%{public}d, "
        "currentServer userId=%{public}d screenId=%{public}d pid=%{public}d, isWMSConnected=%{public}d",
        userId_, shouldUpdateCurrentState, isUserSwitched,
        currentServer_.userId, currentServer_.screenId, currentServer_.pid, isWMSConnected_);
    if (isConnected && isUserSwitched) {
        OnWMSConnectionChangedCallback(fromUserId, screenId, false, fromPid);
        if (userId_ == INVALID_USER_ID && shouldUpdateCurrentState) {
            OnUserSwitch(sessionManagerService);
        }
    }

    OnWMSConnectionChangedCallback(userId, screenId, isConnected, pid);
}

void SessionManager::ClearSessionManagerProxy()
{
    TLOGI(WmsLogTag::WMS_SCB, "begin clear proxy");
    {
        std::lock_guard<std::mutex> lock(sessionManagerServiceMutex_);
        if (sessionManagerServiceProxy_ != nullptr) {
            int refCount = sessionManagerServiceProxy_->GetSptrRefCount();
            TLOGI(WmsLogTag::WMS_SCB, "ref count: %{public}d", refCount);
            sessionManagerServiceProxy_ = nullptr;
        }
    }
    {
        std::lock_guard<std::mutex> lock(sceneSessionManagerMutex_);
        sceneSessionManagerProxy_ = nullptr;
    }
}

__attribute__((no_sanitize("cfi"))) sptr<ISceneSessionManager> SessionManager::GetSceneSessionManagerProxy()
{
    InitSessionManagerServiceProxy();
    InitSceneSessionManagerProxy();
    TLOGI(WmsLogTag::WMS_SCB, "init first time, userId=%{public}d", userId_);
    {
        std::lock_guard<std::mutex> lock(sceneSessionManagerMutex_);
        if (sceneSessionManagerProxy_ != nullptr) {
            return sceneSessionManagerProxy_;
        }
    }
    // Fix the issue where proxy returns null because thread B clears proxy immediately after thread A initializes it
    TLOGW(WmsLogTag::WMS_SCB, "sceneSessionManagerProxy_ is nullptr, try again, userId=%{public}d", userId_);
    InitSessionManagerServiceProxy();
    InitSceneSessionManagerProxy();
    TLOGW(WmsLogTag::WMS_SCB, "sceneSessionManagerProxy_ is nullptr, try again end");
    std::lock_guard<std::mutex> lock(sceneSessionManagerMutex_);
    return sceneSessionManagerProxy_;
}

void SessionManager::InitSessionManagerServiceProxy()
{
    {
        std::lock_guard<std::mutex> lock(sessionManagerServiceMutex_);
        if (sessionManagerServiceProxy_) {
            return;
        }
    }
    if (InitMockSMSProxy() != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SCB, "init failed, userId=%{public}d", userId_);
        return;
    }

    RegisterSMSRecoverListener();

    auto proxy = SessionManagerLite::GetInstance(userId_).GetSessionManagerServiceProxy();
    std::lock_guard<std::mutex> lock(sessionManagerServiceMutex_);
    sessionManagerServiceProxy_ = proxy;
    if (!sessionManagerServiceProxy_) {
        TLOGE(WmsLogTag::WMS_SCB, "failed to get sms proxy, userId=%{public}d", userId_);
    }
}

WMError SessionManager::InitMockSMSProxy()
{
    {
        std::lock_guard<std::mutex> lock(mockSessionManagerServiceMutex_);
        if (mockSessionManagerServiceProxy_) {
            TLOGD(WmsLogTag::DEFAULT, "Mock sms proxy already inited");
            return WMError::WM_OK;
        }
    }
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        TLOGE(WmsLogTag::DEFAULT, "Get SA manager failed, userId=%{public}d", userId_);
        return WMError::WM_ERROR_NULLPTR;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(WINDOW_MANAGER_SERVICE_ID);
    if (!remoteObject) {
        TLOGE(WmsLogTag::DEFAULT, "Get remote from SA failed, userId=%{public}d", userId_);
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lock(mockSessionManagerServiceMutex_);
        mockSessionManagerServiceProxy_ = iface_cast<IMockSessionManagerInterface>(remoteObject);
        if (!mockSessionManagerServiceProxy_) {
            TLOGE(WmsLogTag::DEFAULT, "IPC convert failed, userId=%{public}d", userId_);
            return WMError::WM_ERROR_NULLPTR;
        }
    }
    // Note: Only u0 app should add foundation death recipient.
    if (GetUserIdByUid(getuid()) != SYSTEM_USERID) {
        return WMError::WM_OK;
    }
    sptr<FoundationDeathRecipient> deathRecipient = nullptr;
    {
        std::lock_guard<std::mutex> lock(mockSessionManagerServiceMutex_);
        if (!mockFoundationDeathRecipient_) {
            mockFoundationDeathRecipient_ = sptr<FoundationDeathRecipient>::MakeSptr(userId_);
        }
        deathRecipient = mockFoundationDeathRecipient_;
    }
    if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(deathRecipient)) {
        TLOGE(WmsLogTag::DEFAULT, "Add death recipient failed, userId=%{public}d", userId_);
        {
            std::lock_guard<std::mutex> lock(mockSessionManagerServiceMutex_);
            mockSessionManagerServiceProxy_ = nullptr;
        }
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

__attribute__((no_sanitize("cfi"))) void SessionManager::InitSceneSessionManagerProxy()
{
    TLOGI(WmsLogTag::WMS_SCB, "enter, userId=%{public}d", userId_);
    {
        std::lock_guard<std::mutex> lock(sceneSessionManagerMutex_);
        if (sceneSessionManagerProxy_) {
            return;
        }
    }
    sptr<IRemoteObject> remoteObject = nullptr;
    {
        std::lock_guard<std::mutex> lock(sessionManagerServiceMutex_);
        if (sessionManagerServiceProxy_ == nullptr) {
            TLOGE(WmsLogTag::WMS_SCB, "sms proxy is null, userId=%{public}d", userId_);
            return;
        }
        remoteObject = sessionManagerServiceProxy_->GetSceneSessionManager();
        if (remoteObject == nullptr) {
            TLOGE(WmsLogTag::WMS_SCB, "get sms proxy failed, userId=%{public}d", userId_);
            return;
        }
    }
    if (!sceneSessionManagerDeath_) {
        sceneSessionManagerDeath_ = sptr<SSMDeathRecipient>::MakeSptr(userId_);
    }
    {
        std::lock_guard<std::mutex> lock(sceneSessionManagerMutex_);
        sceneSessionManagerProxy_ = iface_cast<ISceneSessionManager>(remoteObject);
        if (sceneSessionManagerProxy_ == nullptr) {
            TLOGE(WmsLogTag::WMS_SCB, "Get scene session manager proxy failed, userId=%{public}d", userId_);
            return;
        }
    }
    if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(sceneSessionManagerDeath_)) {
        TLOGE(WmsLogTag::WMS_SCB, "failed to add death recipient, userId=%{public}d", userId_);
        return;
    }
}

void SessionManager::RegisterSMSRecoverListener()
{
    {
        std::lock_guard<std::mutex> lock(recoverListenerMutex_);
        if (isRecoverListenerRegistered_) {
            TLOGI(WmsLogTag::WMS_RECOVER, "listener has been registered");
            return;
        }
        if (!smsRecoverListener_) {
            smsRecoverListener_ = sptr<SessionManagerServiceRecoverListener>::MakeSptr(userId_);
        }
    }
    sptr<IMockSessionManagerInterface> mockProxy = nullptr;
    {
        std::lock_guard<std::mutex> lock(mockSessionManagerServiceMutex_);
        if (mockSessionManagerServiceProxy_ == nullptr) {
            TLOGE(WmsLogTag::WMS_SCB, "get mock sms proxy failed, userId=%{public}d", userId_);
            return;
        }
        mockProxy = mockSessionManagerServiceProxy_;
    }
    mockProxy->RegisterSMSRecoverListener(userId_, false, smsRecoverListener_);
    {
        std::lock_guard<std::mutex> lock(recoverListenerMutex_);
        isRecoverListenerRegistered_ = true;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    IPCSkeleton::SetCallingIdentity(identity);
}

void SessionManager::UnregisterSMSRecoverListener()
{
    sptr<IMockSessionManagerInterface> mockProxy = nullptr;
    {
        std::lock_guard<std::mutex> lock(mockSessionManagerServiceMutex_);
        mockProxy = mockSessionManagerServiceProxy_;
    }
    if (mockProxy) {
        mockProxy->UnregisterSMSRecoverListener(userId_, false);
    }
    {
        std::lock_guard<std::mutex> lock(recoverListenerMutex_);
        isRecoverListenerRegistered_ = false;
        smsRecoverListener_ = nullptr;
    }
    TLOGI(WmsLogTag::WMS_RECOVER, "end, userId: %{public}d", userId_);
}

void SessionManager::RegisterWindowManagerRecoverCallbackFunc(const WindowManagerRecoverCallbackFunc& callbackFunc)
{
    TLOGD(WmsLogTag::WMS_RECOVER, "enter");
    std::lock_guard<std::mutex> lock(wmRecoverCallbackMutex_);
    windowManagerRecoverFunc_ = callbackFunc;
}

void SessionManager::RecoverSessionManagerService(const sptr<ISessionManagerService>& sessionManagerService)
{
    {
        std::lock_guard<std::mutex> lock(sessionManagerServiceMutex_);
        sessionManagerServiceProxy_ = sessionManagerService;
    }
    WindowManagerRecoverCallbackFunc callbackFunc = nullptr;
    {
        std::lock_guard<std::mutex> lock(wmRecoverCallbackMutex_);
        callbackFunc = windowManagerRecoverFunc_;
    }
    if (callbackFunc) {
        TLOGI(WmsLogTag::WMS_RECOVER, "begin recover, userId=%{public}d", userId_);
        callbackFunc();
    } else {
        TLOGE(WmsLogTag::WMS_RECOVER, "callback func is null, userId=%{public}d", userId_);
    }
}

void SessionManager::OnUserSwitch(const sptr<ISessionManagerService>& sessionManagerService)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "enter, userId=%{public}d", userId_);
    RemoveSSMDeathRecipient();
    ClearSessionManagerProxy();
    {
        std::lock_guard<std::mutex> lock(sessionManagerServiceMutex_);
        sessionManagerServiceProxy_ = sessionManagerService;
    }
    InitSceneSessionManagerProxy();
    {
        std::lock_guard<std::mutex> lock(sceneSessionManagerMutex_);
        if (sceneSessionManagerProxy_ == nullptr) {
            TLOGE(WmsLogTag::WMS_MULTI_USER,
                "OnUserSwitch init ssm failed, inst=%{public}d", userId_);
            return;
        }
    }

    UserSwitchCallbackFunc callbackFunc = nullptr;
    {
        std::lock_guard<std::mutex> lock(userSwitchCallbackFuncMutex_);
        callbackFunc = userSwitchCallbackFunc_;
    }
    if (callbackFunc) {
        TLOGI(WmsLogTag::WMS_MULTI_USER, "run callback func, userId=%{public}d", userId_);
        callbackFunc();
    } else {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "callback func is null, userId=%{public}d", userId_);
    }
}

void SessionManager::RemoveSSMDeathRecipient()
{
    sptr<IRemoteObject> remoteObject = nullptr;
    {
        std::lock_guard<std::mutex> lock(sceneSessionManagerMutex_);
        if (sceneSessionManagerProxy_) {
            remoteObject = sceneSessionManagerProxy_->AsObject();
        }
    }
    if (remoteObject) {
        remoteObject->RemoveDeathRecipient(sceneSessionManagerDeath_);
    }
    TLOGI(WmsLogTag::WMS_SCB, "removed, userId=%{public}d", userId_);
}

void SessionManager::RemoveMockFoundationDeathRecipient()
{
    sptr<IRemoteObject> remoteObject = nullptr;
    sptr<FoundationDeathRecipient> deathRecipient = nullptr;
    {
        std::lock_guard<std::mutex> lock(mockSessionManagerServiceMutex_);
        if (!mockSessionManagerServiceProxy_ || !mockSessionManagerServiceProxy_->AsObject()) {
            TLOGE(WmsLogTag::DEFAULT, "Remove mock foundation death recipient failed");
            return;
        }
        remoteObject = mockSessionManagerServiceProxy_->AsObject();
        deathRecipient = mockFoundationDeathRecipient_;
    }
    remoteObject->RemoveDeathRecipient(deathRecipient);
    TLOGI(WmsLogTag::DEFAULT, "Removed success");
}

sptr<IMockSessionManagerInterface> SessionManager::GetMockSessionManagerServiceProxy()
{
    std::lock_guard<std::mutex> lock(mockSessionManagerServiceMutex_);
    return mockSessionManagerServiceProxy_;
}

WMError SessionManager::RegisterWMSConnectionChangedListener(const WMSConnectionChangedCallbackFunc& callbackFunc)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "register wms connection changed listener");
    auto ret = WMError::WM_OK;
    bool isWMSAlreadyConnected = false;
    int32_t userId = INVALID_USER_ID;
    int32_t screenId = DEFAULT_SCREEN_ID;
    int32_t pid = INVALID_PID;

    if (callbackFunc == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "callbackFunc is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lock(wmsConnectionMutex_);
        wmsConnectionChangedFunc_ = callbackFunc;
        isWMSAlreadyConnected = isWMSConnected_ && (currentServer_.userId > INVALID_USER_ID);
        userId = currentServer_.userId;
        screenId = currentServer_.screenId;
        pid = currentServer_.pid;
    }
    if (isWMSAlreadyConnected) {
        TLOGI(WmsLogTag::WMS_MULTI_USER, "WMS already connected, notify immediately");
        OnWMSConnectionChangedCallback(userId, screenId, true, pid);
    }

    ret = InitMockSMSProxy();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Init mock session manager service failed");
        return ret;
    }
    RegisterSMSRecoverListener();
    return WMError::WM_OK;
}

WMError SessionManager::UnregisterWMSConnectionChangedListener()
{
    {
        std::lock_guard<std::mutex> lock(wmsConnectionMutex_);
        wmsConnectionChangedFunc_ = nullptr;
    }
    UnregisterSMSRecoverListener();
    return WMError::WM_OK;
}

void SessionManager::RegisterUserSwitchListener(const UserSwitchCallbackFunc& callbackFunc)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "enter, userId=%{public}d", userId_);
    std::lock_guard<std::mutex> lock(userSwitchCallbackFuncMutex_);
    userSwitchCallbackFunc_ = callbackFunc;
}

void SessionManager::OnFoundationDied()
{
    {
        std::lock_guard<std::mutex> lock(wmsConnectionMutex_);
        isWMSConnected_ = false;
        currentServer_ = { INVALID_USER_ID, INVALID_SCREEN_ID_INT32, INVALID_PID };
    }
    {
        std::lock_guard<std::mutex> lock(recoverListenerMutex_);
        isRecoverListenerRegistered_ = false;
        smsRecoverListener_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(sessionManagerServiceMutex_);
        sessionManagerServiceProxy_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(sceneSessionManagerMutex_);
        sceneSessionManagerProxy_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(mockSessionManagerServiceMutex_);
        mockSessionManagerServiceProxy_ = nullptr;
    }
    TLOGI(WmsLogTag::DEFAULT, "Clear success, userId=%{public}d", userId_);
}

void SessionManager::NotifySetSpecificWindowZIndex()
{
    TLOGI(WmsLogTag::WMS_FOCUS, "enter, userId=%{public}d", userId_);
    sptr<IMockSessionManagerInterface> mockProxy = nullptr;
    {
        std::lock_guard<std::mutex> lock(mockSessionManagerServiceMutex_);
        mockProxy = mockSessionManagerServiceProxy_;
    }
    if (mockProxy) {
        mockProxy->NotifySetSpecificWindowZIndex();
    }
}

FoundationDeathRecipient::FoundationDeathRecipient(int32_t userId) : userId_(userId) {}

void FoundationDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "mock sms proxy died, userId_: %{public}d", userId_);
    SessionManager::GetInstance(userId_).OnFoundationDied();
}

SSMDeathRecipient::SSMDeathRecipient(int32_t userId) : userId_(userId) {}

void SSMDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "ssm proxy died, userId_: %{public}d", userId_);
    SessionManager::GetInstance(userId_).RemoveSSMDeathRecipient();
    SessionManager::GetInstance(userId_).ClearSessionManagerProxy();
    TLOGI(WmsLogTag::WMS_RECOVER, "ssm proxy died, clear end");
}
} // namespace OHOS::Rosen
