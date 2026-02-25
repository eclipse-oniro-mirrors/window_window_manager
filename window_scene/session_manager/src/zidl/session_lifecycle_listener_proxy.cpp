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

#include "hilog_tag_wrapper.h"
#include "session_manager/include/zidl/session_lifecycle_listener_proxy.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
void SessionLifecycleListenerProxy::OnLifecycleEvent(SessionLifecycleEvent event, const LifecycleEventPayload& payload)
{
    SendRequestCommon(event, payload);
}

void SessionLifecycleListenerProxy::SendRequestCommon(
    SessionLifecycleEvent event, const LifecycleEventPayload& payload)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(ISessionLifecycleListener::GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interface token failed.");
        return;
    }
    if (!data.WriteInt32(event)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write event faild.");
        return;
    }
    if (!data.WriteParcelable(&payload)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write payload failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        ISessionLifecycleListenerMessage::TRANS_ON_LIFECYCLE_EVENT), data, reply, option) != NO_ERROR) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed.");
        return;
    }
}
}