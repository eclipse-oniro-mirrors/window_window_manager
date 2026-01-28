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

#ifndef OHOS_ROSEN_MOCK_SESSION_MANAGER_SERVICE_H
#define OHOS_ROSEN_MOCK_SESSION_MANAGER_SERVICE_H

#include <shared_mutex>
#include <system_ability.h>

#include "dm_common.h"
#include "mock_session_manager_interface_stub.h"
#include "wm_single_instance.h"
#include "zidl/session_manager_service_recover_interface.h"
#include "scene_session_manager_lite_interface.h"
#include "scene_session_manager_interface.h"

namespace OHOS {
namespace Rosen {
using SMSRecoverListenerMap = std::map<int32_t, sptr<ISessionManagerServiceRecoverListener>>;
class MockSessionManagerService : public SystemAbility, public MockSessionManagerInterfaceStub {
DECLARE_SYSTEM_ABILITY(MockSessionManagerService);
WM_DECLARE_SINGLE_INSTANCE_BASE(MockSessionManagerService);
public:
    ErrCode GetScreenSessionManagerLite(sptr<IRemoteObject>& screenSessionManagerLite) override;
    sptr<IRemoteObject> GetSceneSessionManager();
    void OnStart() override;
    int Dump(int fd, const std::vector<std::u16string>& args) override;
    void GetProcessSurfaceNodeIdByPersistentId(const int32_t pid,
        const std::vector<uint64_t>& windowIdList, std::vector<uint64_t>& surfaceNodeIds);
    void AddSkipSelfWhenShowOnVirtualScreenList(const std::vector<int32_t>& persistentIds, int32_t userId);
    void RemoveSkipSelfWhenShowOnVirtualScreenList(const std::vector<int32_t>& persistentIds, int32_t userId);
    void SetScreenPrivacyWindowTagSwitch(
        uint64_t screenId, const std::vector<std::string>& privacyWindowTags, bool enable);

    /*
     * Multi User
     */
    bool SetSessionManagerService(const sptr<IRemoteObject>& sessionManagerService);
    ErrCode GetSessionManagerService(sptr<IRemoteObject>& sessionManagerService) override;
    ErrCode GetSessionManagerServiceByUserId(int32_t userId, sptr<IRemoteObject>& sessionManagerService) override;
    void NotifyWMSConnected(int32_t userId, DisplayId screenId, bool isColdStart);
    void NotifyNotKillService() {}
    ErrCode GetSceneSessionManagerLiteByClient(int32_t userId, sptr<IRemoteObject>& sceneSessionManagerLite) override;
    ErrCode GetSceneSessionManagerByClient(int32_t userId, sptr<IRemoteObject>& sceneSessionManager) override;
    sptr<ISceneSessionManagerLite> GetSceneSessionManagerLiteBySA(int32_t userId);
    sptr<ISceneSessionManager> GetSceneSessionManagerBySA(int32_t userId);

    /*
     * Window Recover
     */
    ErrCode NotifySceneBoardAvailable() override;
    ErrCode RegisterSMSRecoverListener(const sptr<IRemoteObject>& listener, int32_t userId, bool isLite) override;
    ErrCode UnregisterSMSRecoverListener(int32_t userId, bool isLite) override;
    void UnregisterSMSRecoverListenerInner(int32_t clientUserId, DisplayId displayId, int32_t pid, bool isLite);

    /*
     * Window Hierarchy
     */
    ErrCode NotifySetSpecificWindowZIndex() override;
    void ResetSpecificWindowZIndex(int32_t clientUserId, int32_t pid);

    /*
     * Window Snapshot
     */
    ErrCode SetSnapshotSkipByUserIdAndBundleNames(int32_t userId,
        const std::vector<std::string>& bundleNameList) override;
    ErrCode SetSnapshotSkipByIdNamesMap(const std::unordered_map<int32_t,
        std::vector<std::string>>& userIdAndBunldeNames) override;

protected:
    MockSessionManagerService();
    virtual ~MockSessionManagerService();

private:
    /*
     * Multi User
     */
    class SMSDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit SMSDeathRecipient(int32_t userId);
        void OnRemoteDied(const wptr<IRemoteObject>& object) override;
        bool IsSceneBoardTestMode();
        void SetScreenId(int32_t screenId);
        bool needKillService_ { true };

    private:
        int32_t userId_;
        int32_t screenId_;
    };
    sptr<SMSDeathRecipient> GetSMSDeathRecipientByUserId(int32_t userId);
    void RemoveSMSDeathRecipientByUserId(int32_t userId);
    virtual sptr<IRemoteObject> GetSessionManagerServiceInner(int32_t userId);
    void RemoveSessionManagerServiceByUserId(int32_t userId);
    bool RegisterMockSessionManagerService();
    ErrCode GetForegroundOsAccountDisplayId(int32_t userId, DisplayId& displayId) const;
    ErrCode NotifyWMSConnectionStatus(int32_t userId, const sptr<ISessionManagerServiceRecoverListener>& smsListener);
    virtual int32_t GetUserIdByCallingUid();

    SMSRecoverListenerMap* GetSMSRecoverListenerMap(int32_t userId, bool isLite);
    SMSRecoverListenerMap* GetSystemAppSMSRecoverListenerMap(DisplayId displayId, bool isLite);

    void NotifySceneBoardAvailableToClient(int32_t userId, bool isLite);
    void NotifySceneBoardAvailableToSystemAppClient(int32_t userId, bool isLite);

    void NotifyWMSConnectionChanged(int32_t wmsUserId, DisplayId screenId, bool isConnected);
    void NotifyWMSConnectionChangedToClient(int32_t wmsUserId, DisplayId screenId, bool isConnected, bool isLite);

    int DumpSessionInfo(const std::vector<std::string>& args, std::string& dumpInfo);
    void ShowHelpInfo(std::string& dumpInfo);
    void ShowAceDumpHelp(std::string& dumpInfo);
    void ShowIllegalArgsInfo(std::string& dumpInfo);
    virtual sptr<IRemoteObject> GetSceneSessionManagerInner(int32_t userId, bool isLite);
    virtual ErrCode CheckClientIsSystemUser();
    sptr<IRemoteObject> GetSceneSessionManagerFromCache(int32_t userId, bool isLite);
    void UpdateSceneSessionManagerFromCache(int32_t userId, bool isLite, sptr<IRemoteObject>& sceneSessionManager);
    template <typename T>
    ErrCode GetSceneSessionManagerByUserIdImpl(int32_t userId, sptr<T>& result, bool isLite, bool checkClient);
    void RemoveFromMap(std::map<int32_t, sptr<IRemoteObject>>& map, std::mutex& mutex, int32_t userId);

    /*
     * Window Snapshot
     */
    virtual sptr<IRemoteObject> GetSceneSessionManagerByUserId(int32_t userId);
    ErrCode RecoverSCBSnapshotSkipByUserId(int32_t userId);
    virtual ErrCode NotifySCBSnapshotSkipByUserIdAndBundleNames(int32_t userId,
        const std::vector<std::string>& bundleNameList, const sptr<IRemoteObject>& remoteObject);
    ErrCode SetSnapshotSkipByUserIdAndBundleNamesInner(int32_t userId,
        const std::vector<std::string>& bundleNameList);
    ErrCode SetSnapshotSkipByIdNamesMapInner(const std::unordered_map<int32_t,
        std::vector<std::string>>& userIdAndBunldeNames);

    sptr<IRemoteObject> screenSessionManager_;
    std::mutex screenSessionManagerMutex_;
    sptr<IRemoteObject> defaultSceneSessionManager_;
    std::mutex defaultSceneSessionManagerMutex_;

    /*
     * Multi User
     */
    int32_t defaultWMSUserId_;
    std::mutex defaultWMSUserIdMutex_;
    DisplayId defaultScreenId_;
    std::mutex userId2ScreenIdMapMutex_;
    std::map<int32_t, DisplayId> userId2ScreenIdMap_;
    std::shared_mutex smsDeathRecipientMapLock_;
    std::map<int32_t, sptr<SMSDeathRecipient>> smsDeathRecipientMap_;
    std::mutex sessionManagerServiceMapMutex_;
    std::map<int32_t, sptr<IRemoteObject>> sessionManagerServiceMap_;
    std::mutex wmsConnectionStatusLock_;
    std::map<int32_t, bool> wmsConnectionStatusMap_;
    std::mutex sceneSessionManagerLiteMapMutex_;
    std::map<int32_t, sptr<IRemoteObject>> sceneSessionManagerLiteMap_;
    std::mutex sceneSessionManagerMapMutex_;
    std::map<int32_t, sptr<IRemoteObject>> sceneSessionManagerMap_;

    /*
     * Window Recover
     */
    std::map<int32_t, SMSRecoverListenerMap> recoverListenerMap_;
    std::mutex recoverListenerMutex_;
    std::map<int32_t, SMSRecoverListenerMap> liteRecoverListenerMap_;
    std::mutex liteRecoverListenerMutex_;

    std::map<DisplayId, SMSRecoverListenerMap> systemAppRecoverListenerMap_;
    std::mutex systemAppRecoverListenerMutex_;
    std::map<DisplayId, SMSRecoverListenerMap> liteSystemAppRecoverListenerMap_;
    std::mutex liteSystemAppRecoverListenerMutex_;

    /*
     * Window Hierarchy
     */
    std::unordered_map<int32_t, int32_t> specificZIndexByPidMap_;
    std::mutex specificZIndexByPidMapMutex_;

    /*
     * Window Snapshot
     */
    std::mutex userIdBundleNamesMapLock_;
    std::unordered_map<int32_t, std::vector<std::string>> userIdBundleNamesMap_;
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_MOCK_SESSION_MANAGER_SERVICE_H
