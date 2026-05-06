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

#ifndef OHOS_SESSION_LIFECYCLE_LISTENER_STUB_H
#define OHOS_SESSION_LIFECYCLE_LISTENER_STUB_H

#include <iremote_stub.h>
#include <iremote_object.h>

#include "session_lifecycle_listener_interface.h"

namespace OHOS::Rosen {

class SessionLifecycleListenerStub : public IRemoteStub<ISessionLifecycleListener> {
public:
    SessionLifecycleListenerStub() = default;
    virtual ~SessionLifecycleListenerStub() = default;

    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

private:
    int HandleOnLifecycleEvent(MessageParcel& data, MessageParcel& reply);
    int HandleOnAppInstanceLifecycleEvent(MessageParcel& data, MessageParcel& reply);
    int HandleOnBatchLifecycleEvent(MessageParcel& data, MessageParcel& reply);
};
} // namespace OHOS
#endif //OHOS_SESSION_LIFECYCLE_LISTENER_STUB_H