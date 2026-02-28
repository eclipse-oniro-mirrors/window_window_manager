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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_H
#define OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_H

#include <functional>
#include <iremote_stub.h>

#include "imock_session_manager_interface.h"
#include "session_manager_service_interface.h"
#include "session_manager_service_recover_interface.h"
#include "wm_common.h"
#include "wm_single_instance.h"
#include "zidl/scene_session_manager_lite_interface.h"
#include "zidl/screen_session_manager_lite_interface.h"

namespace OHOS::Rosen {
class SessionManagerLite;
class SessionManagerServiceLiteRecoverListener : public IRemoteStub<ISessionManagerServiceRecoverListener> {
public:
    explicit SessionManagerServiceLiteRecoverListener(int32_t userId);
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnSessionManagerServiceRecover(const sptr<IRemoteObject>& sessionManagerService) override;
    void OnWMSConnectionChanged(int32_t wmsUserId,
                                int32_t screenId,
                                bool isConnected,
                                const sptr<IRemoteObject>& sessionManagerService) override;

private:
    const int32_t userId_;
};

class SSMDeathRecipientLite : public IRemoteObject::DeathRecipient {
public:
    explicit SSMDeathRecipientLite(const int32_t userId = INVALID_USER_ID);
    void OnRemoteDied(const wptr<IRemoteObject>& wptrDeath) override;

private:
    const int32_t userId_;
};

class FoundationDeathRecipientLite : public IRemoteObject::DeathRecipient {
public:
    explicit FoundationDeathRecipientLite(const int32_t userId = INVALID_USER_ID);
    void OnRemoteDied(const wptr<IRemoteObject>& wptrDeath) override;

private:
    const int32_t userId_;
};

class SessionManagerLite : public RefBase {
    WM_DECLARE_SINGLE_INSTANCE_BASE(SessionManagerLite);
public:
    static SessionManagerLite& GetInstance(const int32_t userId);

    void ClearSessionManagerProxy();
    void RemoveSSMDeathRecipient();

    sptr<ISceneSessionManagerLite> GetSceneSessionManagerLiteProxy();
    sptr<IScreenSessionManagerLite> GetScreenSessionManagerLiteProxy();

    sptr<ISessionManagerService> GetSessionManagerServiceProxy();

    void SaveSessionListener(const sptr<ISessionListener>& listener);
    void DeleteSessionListener(const sptr<ISessionListener>& listener);

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
    void RecoverSessionManagerService(const sptr<ISessionManagerService>& sessionManagerService);

private:
    friend class sptr<SessionManagerLite>;
    SessionManagerLite(const int32_t userId = INVALID_USER_ID);
    ~SessionManagerLite() override;

    virtual WMError InitSessionManagerServiceProxy();
    virtual WMError InitSceneSessionManagerLiteProxy();
    virtual WMError InitScreenSessionManagerLiteProxy();

    void DeleteAllSessionListeners();
    void ReregisterSessionListener();

    virtual sptr<IMockSessionManagerInterface> GetMockSessionManagerServiceProxy();

    /*
     * Multi User and multi screen
     */
    const int32_t userId_;
    static std::unordered_map<int32_t, sptr<SessionManagerLite>> sessionManagerLiteMap_;
    static std::mutex sessionManagerLiteMapMutex_;
    void OnUserSwitch(const sptr<ISessionManagerService>& sessionManagerService);
    void OnWMSConnectionChangedCallback(int32_t userId, int32_t screenId, bool isConnected);

    /*
     * Window Recover
     */
    virtual WMError RegisterSMSRecoverListener();
    void UnregisterSMSRecoverListener();

    sptr<FoundationDeathRecipientLite> mockFoundationDeathRecipient_ = nullptr;
    sptr<IMockSessionManagerInterface> mockSessionManagerServiceProxy_ = nullptr;
    std::mutex mockSMSProxyMutex_;

    sptr<IRemoteObject> smsRecoverListener_ = nullptr;
    bool isRecoverListenerRegistered_ = false;
    std::mutex smsRecoverListenerMutex_;

    sptr<ISessionManagerService> sessionManagerServiceProxy_ = nullptr;
    std::mutex smsProxyMutex_;

    sptr<SSMDeathRecipientLite> ssmLiteDeathRecipient_ = nullptr;
    sptr<ISceneSessionManagerLite> sceneSessionManagerLiteProxy_ = nullptr;
    std::mutex ssmLiteProxyMutex_;

    sptr<IScreenSessionManagerLite> screenSessionManagerLiteProxy_ = nullptr;
    std::mutex screenSMLiteProxyMutex_;

    std::recursive_mutex listenerLock_;
    std::vector<sptr<ISessionListener>> sessionListeners_;
    // above guarded by listenerLock_

    /*
     * Multi User
     */
    UserSwitchCallbackFunc userSwitchCallbackFunc_ = nullptr;
    std::mutex userSwitchCallbackFuncMutex_;

    std::mutex wmsConnectionMutex_;
    int32_t currentWMSUserId_ = INVALID_USER_ID;
    int32_t currentScreenId_ = DEFAULT_SCREEN_ID;
    bool isWMSConnected_ = false;
    WMSConnectionChangedCallbackFunc wmsConnectionChangedFunc_ = nullptr;
    // above guarded by wmsConnectionMutex_
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_H
