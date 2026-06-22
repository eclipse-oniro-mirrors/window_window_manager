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

#ifndef OHOS_SESSION_PIP_CHANGE_LISTENER_PROXY_H
#define OHOS_SESSION_PIP_CHANGE_LISTENER_PROXY_H

#include <iremote_proxy.h>
#include "pip_change_listener.h"

namespace OHOS::Rosen {
class PipChangeListenerProxy : public IRemoteProxy<IPipChangeListener> {
public:
    explicit PipChangeListenerProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy(impl) {}

    ~PipChangeListenerProxy() override = default;

    void OnPipStart(int32_t windowId) override;

private:
    static inline BrokerDelegator<PipChangeListenerProxy> delegator_;
};
} // namespace OHOS
#endif //OHOS_SESSION_PIP_CHANGE_LISTENER_PROXY_H