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

#include <cinttypes>
#include "zidl/mock_session_manager_service_proxy.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "MockSessionManagerServiceProxy"};
}

sptr<IRemoteObject> MockSessionManagerServiceProxy::GetSessionManagerService()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return nullptr;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        MockSessionManagerServiceMessage::TRANS_ID_GET_SESSION_MANAGER_SERVICE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = reply.ReadRemoteObject();
    return remoteObject;
}


sptr<IRemoteObject> MockSessionManagerServiceProxy::GetScreenSessionManagerLite()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return nullptr;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        MockSessionManagerServiceMessage::TRANS_ID_GET_SCREEN_SESSION_MANAGER),
        data, reply, option) != ERR_NONE) {
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = reply.ReadRemoteObject();
    return remoteObject;
}

void MockSessionManagerServiceProxy::NotifySceneBoardAvailable()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_RECOVER, "WriteInterfaceToken failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        MockSessionManagerServiceMessage::TRANS_ID_NOTIFY_SCENE_BOARD_AVAILABLE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_RECOVER, "SendRequest failed");
        return;
    }
}

void MockSessionManagerServiceProxy::RegisterSMSRecoverListener(const sptr<IRemoteObject>& listener)
{
    TLOGD(WmsLogTag::WMS_RECOVER, "Register");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_RECOVER, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteRemoteObject(listener)) {
        TLOGE(WmsLogTag::WMS_RECOVER, "WriteRemoteObject listener failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        MockSessionManagerServiceMessage::TRANS_ID_REGISTER_SMS_RECOVER_LISTENER),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_RECOVER, "SendRequest failed");
        return;
    }
}

void MockSessionManagerServiceProxy::UnregisterSMSRecoverListener()
{
    TLOGD(WmsLogTag::WMS_RECOVER, "UnRegister");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_RECOVER, "WriteInterfaceToken failed");
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        MockSessionManagerServiceMessage::TRANS_ID_UNREGISTER_SMS_RECOVER_LISTENER),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_RECOVER, "SendRequest failed");
        return;
    }
}

void MockSessionManagerServiceProxy::RegisterSMSLiteRecoverListener(const sptr<IRemoteObject>& listener)
{
    TLOGD(WmsLogTag::WMS_RECOVER, "Register");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_RECOVER, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteRemoteObject(listener)) {
        TLOGE(WmsLogTag::WMS_RECOVER, "WriteRemoteObject listener failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        MockSessionManagerServiceMessage::TRANS_ID_REGISTER_SMS_LITE_RECOVER_LISTENER),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_RECOVER, "SendRequest failed");
        return;
    }
}

void MockSessionManagerServiceProxy::UnregisterSMSLiteRecoverListener()
{
    TLOGD(WmsLogTag::WMS_RECOVER, "Unregister");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_RECOVER, "WriteInterfaceToken failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        MockSessionManagerServiceMessage::TRANS_ID_UNREGISTER_SMS_LITE_RECOVER_LISTENER),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_RECOVER, "SendRequest failed");
        return;
    }
}

} // namespace Rosen
} // namespace OHOS