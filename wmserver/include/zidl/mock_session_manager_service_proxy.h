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

#ifndef OHOS_MOCK_SESSION_MANAGER_PROXY_H
#define OHOS_MOCK_SESSION_MANAGER_PROXY_H

#include <iremote_proxy.h>
#include "mock_session_manager_service_interface.h"

namespace OHOS {
namespace Rosen {
class MockSessionManagerServiceProxy : public IRemoteProxy<IMockSessionManagerInterface> {
public:
    explicit MockSessionManagerServiceProxy(const sptr<IRemoteObject>& impl) :
        IRemoteProxy<IMockSessionManagerInterface>(impl) {};
    ~MockSessionManagerServiceProxy() {};
    sptr<IRemoteObject> GetSessionManagerService() override;
    sptr<IRemoteObject> GetScreenSessionManagerLite() override;
    void NotifySceneBoardAvailable() override;
    void RegisterSMSRecoverListener(const sptr<IRemoteObject>& listener) override;
    void UnregisterSMSRecoverListener() override;
    void RegisterSMSLiteRecoverListener(const sptr<IRemoteObject>& listener) override;
    void UnregisterSMSLiteRecoverListener() override;

private:
    static inline BrokerDelegator<MockSessionManagerServiceProxy> delegator_;
};
}
}
#endif // OHOS_MOCK_SESSION_MANAGER_PROXY_H