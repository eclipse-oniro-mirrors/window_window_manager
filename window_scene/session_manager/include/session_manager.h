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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_H
#define OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_H

#include <iremote_stub.h>
#include "imock_session_manager_interface.h"
#include "session_manager_service_interface.h"
#include "session_manager_service_recover_interface.h"
#include "wm_common.h"
#include "wm_single_instance.h"
#include "zidl/scene_session_manager_interface.h"

namespace OHOS::Rosen {
class SessionManagerServiceRecoverListener : public IRemoteStub<ISessionManagerServiceRecoverListener> {
public:
    explicit SessionManagerServiceRecoverListener(int32_t userId);
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnSessionManagerServiceRecover(const sptr<IRemoteObject>& sessionManagerService) override;
    void OnWMSConnectionChanged(int32_t wmsUserId,
                                int32_t screenId,
                                bool isConnected,
                                const sptr<IRemoteObject>& sessionManagerService) override;

private:
    int32_t userId_;
};

class SSMDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit SSMDeathRecipient(const int32_t userId = INVALID_USER_ID);
    void OnRemoteDied(const wptr<IRemoteObject>& wptrDeath) override;

private:
    int32_t userId_;
};

class FoundationDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit FoundationDeathRecipient(int32_t userId = INVALID_USER_ID);
    void OnRemoteDied(const wptr<IRemoteObject>& wptrDeath) override;

private:
    int32_t userId_;
};

class SessionManager : public RefBase {
    WM_DECLARE_SINGLE_INSTANCE_BASE(SessionManager);

public:
    static SessionManager& GetInstance(const int32_t userId);
    void ClearSessionManagerProxy();
    void RemoveSSMDeathRecipient();

    sptr<ISceneSessionManager> GetSceneSessionManagerProxy();
    void OnFoundationDied();

    /*
     * Multi User
     */
    using WMSConnectionChangedCallbackFunc = std::function<void(int32_t, int32_t, bool)>;
    WMError RegisterWMSConnectionChangedListener(const WMSConnectionChangedCallbackFunc& callbackFunc);
    WMError UnregisterWMSConnectionChangedListener();
    void OnWMSConnectionChanged(
        int32_t userId, int32_t screenId, bool isConnected, const sptr<ISessionManagerService>& sessionManagerService);

    using UserSwitchCallbackFunc = std::function<void()>;
    void RegisterUserSwitchListener(const UserSwitchCallbackFunc& callbackFunc);

    /*
     * Window Recover
     */
    using WindowManagerRecoverCallbackFunc = std::function<void()>;
    void RegisterWindowManagerRecoverCallbackFunc(const WindowManagerRecoverCallbackFunc& callbackFunc);
    void RecoverSessionManagerService(const sptr<ISessionManagerService>& sessionManagerService);

    /*
     * Window Hierarchy
     */
    void NotifySetSpecificWindowZIndex();

private:
    friend class sptr<SessionManager>;
    ~SessionManager() override;
    SessionManager(const int32_t userId = INVALID_USER_ID);

    void InitSessionManagerServiceProxy();
    WMError InitMockSMSProxy();
    void InitSceneSessionManagerProxy();

    /*
     * Window Recover
     */
    void RegisterSMSRecoverListener();
    void UnregisterSMSRecoverListener();

    sptr<FoundationDeathRecipient> foundationDeath_ = nullptr;
    bool isFoundationListenerRegistered_ = false;
    std::mutex foundationListenerRegisterdMutex_;
    sptr<IMockSessionManagerInterface> mockSessionManagerServiceProxy_ = nullptr;
    std::mutex mockSessionManagerServiceMutex_;

    sptr<SSMDeathRecipient> sceneSessionManagerDeath_ = nullptr;
    sptr<ISceneSessionManager> sceneSessionManagerProxy_ = nullptr;
    std::mutex sceneSessionManagerMutex_;

    sptr<ISessionManagerService> sessionManagerServiceProxy_ = nullptr;
    std::mutex sessionManagerServiceMutex_;

    sptr<IRemoteObject> smsRecoverListener_ = nullptr;
    bool isRecoverListenerRegistered_ = false;
    std::mutex recoverListenerMutex_;

    WindowManagerRecoverCallbackFunc windowManagerRecoverFunc_ = nullptr;
    std::mutex wmRecoverCallbackMutex_;

    /*
     * Multi User and multi screen
     */
    void OnWMSConnectionChangedCallback(int32_t userId, int32_t screenId, bool isConnected);
    void OnUserSwitch(const sptr<ISessionManagerService>& sessionManagerService);
    int32_t userId_;
    static std::unordered_map<int32_t, sptr<SessionManager>> sessionManagerMap_;
    static std::mutex sessionManagerMapMutex_;
    UserSwitchCallbackFunc userSwitchCallbackFunc_ = nullptr;
    std::mutex userSwitchCallbackFuncMutex_;

    std::mutex wmsConnectionMutex_;
    bool isWMSConnected_ = false;
    int32_t currentWMSUserId_ = INVALID_USER_ID;
    int32_t currentScreenId_ = DEFAULT_SCREEN_ID;
    WMSConnectionChangedCallbackFunc wmsConnectionChangedFunc_ = nullptr;
    // above guarded by wmsConnectionMutex_, among OnWMSConnectionChanged for wms connection event, user switched,
    // register WMSConnectionChangedListener.
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_H
