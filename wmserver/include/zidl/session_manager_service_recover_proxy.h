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

#ifndef OHOS_SESSION_MANAGER_SERVICE_RECOVER_LISTENER_PROXY_H
#define OHOS_SESSION_MANAGER_SERVICE_RECOVER_LISTENER_PROXY_H

#include <iremote_proxy.h>
#include "session_manager_service_recover_interface.h"

namespace OHOS {
namespace Rosen {
class SessionManagerServiceRecoverProxy : public IRemoteProxy<ISessionManagerServiceRecoverListener> {
public:
    explicit SessionManagerServiceRecoverProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ISessionManagerServiceRecoverListener>(impl) {};
    ~SessionManagerServiceRecoverProxy() = default;
    void OnSessionManagerServiceRecover(const sptr<IRemoteObject>& sessionManagerService) override;
    void OnWMSConnectionChanged(
        int32_t userId, int32_t screenId, bool isConnected,
        const sptr<IRemoteObject>& sessionManagerService, int32_t pid) override;

private:
    static inline BrokerDelegator<SessionManagerServiceRecoverProxy> delegator_;
};
}
}

#endif // OHOS_SESSION_MANAGER_SERVICE_RECOVER_LISTENER_PROXY_H