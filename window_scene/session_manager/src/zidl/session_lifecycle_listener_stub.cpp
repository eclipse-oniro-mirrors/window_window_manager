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

#include <memory>

#include "hilog_tag_wrapper.h"
#include "marshalling_helper.h"
#include "session_manager/include/zidl/session_lifecycle_listener_stub.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {

int SessionLifecycleListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TLOGE(WmsLogTag::WMS_LIFE, "local descriptor not equal to remote.");
        return ERR_INVALID_STATE;
    }
    switch (code) {
        case static_cast<uint32_t>(
            ISessionLifecycleListener::ISessionLifecycleListenerMessage::TRANS_ON_LIFECYCLE_EVENT):
            return HandleOnLifecycleEvent(data, reply);
        case static_cast<uint32_t>(
            ISessionLifecycleListener::ISessionLifecycleListenerMessage::TRANS_ON_APP_INSTANCE_LIFECYCLE_EVENT):
            return HandleOnAppInstanceLifecycleEvent(data, reply);
        case static_cast<uint32_t>(
            ISessionLifecycleListener::ISessionLifecycleListenerMessage::TRANS_ON_BATCH_LIFECYCLE_EVENT):
            return HandleOnBatchLifecycleEvent(data, reply);
        default:
            TLOGE(WmsLogTag::WMS_LIFE, "Failed to handle request, code: %{public}u", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int SessionLifecycleListenerStub::HandleOnLifecycleEvent(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    int32_t event = 0;
    if (!data.ReadInt32(event)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to read event");
        return ERR_INVALID_DATA;
    }
    if (event < static_cast<int32_t>(SessionLifecycleEvent::CREATED) ||
        event >= static_cast<int32_t>(SessionLifecycleEvent::EVENT_END)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Invalid lifecycle event");
        return ERR_INVALID_DATA;
    }
    std::unique_ptr<LifecycleEventPayload> payload(data.ReadParcelable<LifecycleEventPayload>());
    if (!payload) {
        TLOGE(WmsLogTag::WMS_LIFE, "Invalid payload");
        return ERR_INVALID_DATA;
    }
    OnLifecycleEvent(static_cast<SessionLifecycleEvent>(event), *payload);
    return 0;
}

int SessionLifecycleListenerStub::HandleOnAppInstanceLifecycleEvent(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    std::unique_ptr<LifecycleEventPayload> payload(data.ReadParcelable<LifecycleEventPayload>());
    if (!payload) {
        TLOGE(WmsLogTag::WMS_LIFE, "Invalid payload");
        return ERR_INVALID_DATA;
    }
    OnAppInstanceLifecycleEvent(*payload);
    return 0;
}

int SessionLifecycleListenerStub::HandleOnBatchLifecycleEvent(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    std::vector<LifecycleEventPayload> payloads;
    bool res = MarshallingHelper::UnmarshallingVectorObj<LifecycleEventPayload>(
        data, payloads,
        [](Parcel& parcel, LifecycleEventPayload& payload) {
            std::unique_ptr<LifecycleEventPayload> payloadPtr(parcel.ReadParcelable<LifecycleEventPayload>());
            if (!payloadPtr) {
                return false;
            }
            payload = *payloadPtr;
            return true;
        }
    );
    if (!res) {
        TLOGE(WmsLogTag::WMS_LIFE, "Invalid batch payloads");
        return ERR_INVALID_DATA;
    }
    OnBatchLifecycleEvent(payloads);
    return 0;
}
}
