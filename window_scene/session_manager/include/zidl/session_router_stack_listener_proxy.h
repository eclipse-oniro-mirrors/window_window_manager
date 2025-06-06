/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_SESSION_ROUTER_STACK_LISTENER_PROXY_H
#define OHOS_SESSION_ROUTER_STACK_LISTENER_PROXY_H

#include <iremote_proxy.h>
#include "session_router_stack_listener.h"

namespace OHOS::Rosen {
class SessionRouterStackListenerProxy : public IRemoteProxy<ISessionRouterStackListener> {
public:
    explicit SessionRouterStackListenerProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ISessionRouterStackListener>(impl) {}

    virtual ~SessionRouterStackListenerProxy() = default;

    void SendRouterStackInfo(const sptr<RouterStackInfo>& routerStackInfo) override;

private:
    void OnGetRouterStackInfo(const sptr<RouterStackInfo>& routerStackInfo);

    static inline BrokerDelegator<SessionRouterStackListenerProxy> delegator_;
};
} // namespace OHOS
#endif //OHOS_SESSION_ROUTER_STACK_LISTENER_PROXY_H