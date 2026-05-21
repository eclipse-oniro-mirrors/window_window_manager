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

#ifndef OHOS_SESSION_MANAGER_SERVICE_RECOVER_LISTENER_H
#define OHOS_SESSION_MANAGER_SERVICE_RECOVER_LISTENER_H

#include <iremote_broker.h>

namespace OHOS {
namespace Rosen {
class ISessionManagerServiceRecoverListener : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ISessionManagerServiceRecoverListener");
    enum class SessionManagerServiceRecoverMessage : uint32_t {
        TRANS_ID_ON_SESSION_MANAGER_SERVICE_RECOVER,
        TRANS_ID_ON_WMS_CONNECTION_CHANGED,
    };

    /*
     * Window Recover
     */
    virtual void OnSessionManagerServiceRecover(const sptr<IRemoteObject>& sessionManagerService) = 0;

    /*
     * Multi User
     */
    virtual void OnWMSConnectionChanged(
        int32_t userId, int32_t screenId, bool isConnected,
        const sptr<IRemoteObject>& sessionManagerService, int32_t pid) = 0;
};
}
}

#endif // OHOS_SESSION_MANAGER_SERVICE_RECOVER_LISTENER_H