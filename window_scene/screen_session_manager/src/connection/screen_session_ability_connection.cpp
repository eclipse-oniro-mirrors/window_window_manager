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

#include "connection/screen_session_ability_connection.h"

#include <chrono>
#include <want.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "ability_connection.h"
#include "window_manager_hilog.h"
#include "ipc_skeleton.h"
#include "extension_manager_client.h"
#include "dms_global_mutex.h"

namespace OHOS::Rosen {
constexpr int32_t DEFAULT_VALUE = -1;
constexpr int32_t RES_FAILURE = -1;
constexpr int32_t RES_SUCCESS = 0;
constexpr uint32_t SEND_MESSAGE_SYNC_OUT_TIME = 800; // ms
constexpr uint32_t TRANS_RELEASE_BLOCK = 0;
constexpr uint32_t TRANS_FAILED_FOR_PRIVACY = 1;

void ScreenSessionAbilityConnectionStub::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    TLOGI(WmsLogTag::DMS, "OnAbilityConnectDone entry");
    if (resultCode != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "ability connect failed, error code:%{public}d", resultCode);
        return;
    }
    TLOGI(WmsLogTag::DMS, "ability connect success, ability name %{public}s", element.GetAbilityName().c_str());
    if (remoteObject == nullptr) {
        TLOGE(WmsLogTag::DMS, "get remoteObject failed");
        return;
    }
    {
        std::lock_guard<std::mutex> remoteObjLock(remoteObjectMutex_);
        remoteObject_ = remoteObject;
    }
    if (!AddObjectDeathRecipient()) {
        TLOGE(WmsLogTag::DMS, "AddObjectDeathRecipient failed");
        return;
    }

    isConnected_.store(true);
    connectedCv_.notify_all();
    TLOGI(WmsLogTag::DMS, "OnAbilityConnectDone exit");
}

void ScreenSessionAbilityConnectionStub::OnAbilityDisconnectDone(
    const AppExecFwk::ElementName &element, int32_t resultCode)
{
    TLOGI(WmsLogTag::DMS, "OnAbilityDisconnectDone entry");
    if (resultCode != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "ability disconnect failed, error code:%{public}d", resultCode);
    }

    TLOGI(WmsLogTag::DMS, "bundleName:%{public}s, abilityName:%{public}s, resultCode:%{public}d",
        element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);

    std::lock_guard<std::mutex> remoteObjLock(remoteObjectMutex_);
    if (remoteObject_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "remoteObject member is nullptr");
        return;
    }
    remoteObject_->RemoveDeathRecipient(deathRecipient_);
    remoteObject_ = nullptr;
    isConnected_.store(false);
    TLOGI(WmsLogTag::DMS, "OnAbilityDisconnectDone exit");
}

bool ScreenSessionAbilityConnectionStub::AddObjectDeathRecipient()
{
    sptr<ScreenSessionAbilityDeathRecipient> deathRecipient(
        new(std::nothrow) ScreenSessionAbilityDeathRecipient([this] {
        TLOGI(WmsLogTag::DMS, "add death recipient handler");
        sendMessageWaitFlag_ = true;
        blockSendMessageCV_.notify_all();
        TLOGI(WmsLogTag::DMS, "blockSendMessageCV_ notify");
        std::lock_guard<std::mutex> remoteObjLock(remoteObjectMutex_);
        remoteObject_ = nullptr;
        isConnected_.store(false);
    }));

    if (deathRecipient == nullptr) {
        TLOGE(WmsLogTag::DMS, "create ScreenSessionAbilityDeathRecipient failed");
        return false;
    }
    deathRecipient_ = deathRecipient;
    std::lock_guard<std::mutex> remoteObjLock(remoteObjectMutex_);
    if (remoteObject_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "get the remoteObject failed");
        return false;
    }
    if (!remoteObject_->AddDeathRecipient(deathRecipient_)) {
        TLOGE(WmsLogTag::DMS, "AddDeathRecipient failed");
        return false;
    }
    return true;
}

bool ScreenSessionAbilityConnectionStub::IsAbilityConnected()
{
    return isConnected_.load();
}

bool ScreenSessionAbilityConnectionStub::IsAbilityConnectedSync()
{
    std::unique_lock<std::mutex> lock(connectedMutex_);
    if (!isConnected_) {
        DmUtils::safe_wait_for(connectedCv_, lock, std::chrono::milliseconds(SEND_MESSAGE_SYNC_OUT_TIME));
    }
    return IsAbilityConnected();
}

int32_t ScreenSessionAbilityConnectionStub::SendMessageSync(int32_t transCode,
    MessageParcel &data, MessageParcel &reply)
{
    std::unique_lock<std::mutex> lock(connectedMutex_);
    if (!isConnected_) {
        DmUtils::safe_wait_for(connectedCv_, lock, std::chrono::milliseconds(SEND_MESSAGE_SYNC_OUT_TIME));
    }
    if (!IsAbilityConnected()) {
        TLOGE(WmsLogTag::DMS, "ability connection is not established");
        lock.unlock();
        return RES_FAILURE;
    }
    lock.unlock();
    MessageOption option;
    std::lock_guard<std::mutex> remoteObjLock(remoteObjectMutex_);
    if (remoteObject_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "remoteObject is nullptr");
        return RES_FAILURE;
    }
    int32_t ret = remoteObject_->SendRequest(transCode, data, reply, option);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "remoteObject send request failed");
        return RES_FAILURE;
    }
    return RES_SUCCESS;
}

int32_t ScreenSessionAbilityConnectionStub::SendMessageSyncBlock(int32_t transCode,
    MessageParcel &data, MessageParcel &reply)
{
    std::unique_lock<std::mutex> lock(connectedMutex_);
    if (!isConnected_) {
        DmUtils::safe_wait_for(connectedCv_, lock, std::chrono::milliseconds(SEND_MESSAGE_SYNC_OUT_TIME));
    }
    if (!IsAbilityConnected()) {
        TLOGE(WmsLogTag::DMS, "ability connection is not established");
        lock.unlock();
        return RES_FAILURE;
    }
    lock.unlock();
    MessageOption option;
    {
        std::lock_guard<std::mutex> remoteObjLock(remoteObjectMutex_);
        if (remoteObject_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "remoteObject is nullptr");
            return RES_FAILURE;
        }
        if (!data.WriteInterfaceToken(remoteObject_->GetInterfaceDescriptor())) {
            TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
            return RES_FAILURE;
        }
        if (!data.WriteRemoteObject(this->AsObject())) {
            TLOGE(WmsLogTag::DMS, "WriteRemoteObject failed");
            return RES_FAILURE;
        }
        int32_t ret = remoteObject_->SendRequest(transCode, data, reply, option);
        if (ret != ERR_OK) {
            TLOGE(WmsLogTag::DMS, "remoteObject send request failed");
            return RES_FAILURE;
        }
    }

    std::unique_lock<std::mutex> lockSendMessage(sendMessageMutex_);
    TLOGI(WmsLogTag::DMS, "LockSendMessage wait");
    sendMessageWaitFlag_ = false;
    while (!sendMessageWaitFlag_) {
        blockSendMessageCV_.wait(lockSendMessage);
    }

    return RES_SUCCESS;
}

void ScreenSessionAbilityDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remoteObject)
{
    if (remoteObject == nullptr) {
        TLOGE(WmsLogTag::DMS, "remoteObject is null");
        return;
    }

    sptr<IRemoteObject> object = remoteObject.promote();
    if (!object) {
        TLOGE(WmsLogTag::DMS, "object is null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "OnRemoteDied entry");
    if (deathHandler_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "death handler is nullptr");
        return;
    }
    deathHandler_();
}

bool ScreenSessionAbilityConnection::ScreenSessionConnectExtension(
    const std::string &bundleName, const std::string &abilityName)
{
    return ScreenSessionConnectExtension(bundleName, abilityName, {});
}

bool ScreenSessionAbilityConnection::ScreenSessionConnectExtension(const std::string &bundleName,
    const std::string &abilityName, const std::vector<std::pair<std::string, std::string>> &params)
{
    TLOGI(WmsLogTag::DMS, "bundleName:%{public}s, abilityName:%{public}s", bundleName.c_str(), abilityName.c_str());
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetSystemAbilityManager failed");
        return false;
    }
    OHOS::sptr<OHOS::IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetSystemAbility BMS failed");
        return false;
    }
    if (abilityConnectionStub_ != nullptr) {
        TLOGI(WmsLogTag::DMS, "screen session ability extension is already connected");
        return true;
    }
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);
    for (auto param : params) {
        std::string paramKey = param.first;
        std::string paramValue = param.second;
        if (!paramKey.empty() && !paramValue.empty()) {
            want.SetParam(paramKey, paramValue);
            TLOGI(WmsLogTag::DMS, "add want param. paramKey=%{public}s, paramValue=%{public}s",
                paramKey.c_str(), paramValue.c_str());
        }
    }
    abilityConnectionStub_ = sptr<ScreenSessionAbilityConnectionStub>(new (std::nothrow)
        ScreenSessionAbilityConnectionStub());
    if (abilityConnectionStub_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen session aibility connection is nullptr");
        return false;
    }

    // reset current callingIdentity for screen session
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto ret = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(
        want, abilityConnectionStub_, nullptr, DEFAULT_VALUE);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "ConnectServiceExtensionAbility failed, result: %{public}d", ret);
        // set current callingIdentity back
        IPCSkeleton::SetCallingIdentity(identity);
        return false;
    }
    TLOGI(WmsLogTag::DMS, "ConnectServiceExtensionAbility succeed");
    // set current callingIdentity back
    IPCSkeleton::SetCallingIdentity(identity);
    return true;
}

void ScreenSessionAbilityConnection::ScreenSessionDisconnectExtension()
{
    if (abilityConnectionStub_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "ability connect failed");
        return;
    }
    auto ret = AAFwk::ExtensionManagerClient::GetInstance().DisconnectAbility(abilityConnectionStub_);
    if (ret == NO_ERROR) {
        abilityConnectionStub_.clear();
        abilityConnectionStub_ = nullptr;
    }
    TLOGI(WmsLogTag::DMS, "screen session ability disconnected, ret: %{public}d", ret);
}

int32_t ScreenSessionAbilityConnection::SendMessage(
    const int32_t &transCode, MessageParcel &data, MessageParcel &reply)
{
    if (abilityConnectionStub_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "ability connection is nullptr");
        return RES_FAILURE;
    }
    int32_t ret = abilityConnectionStub_->SendMessageSync(transCode, data, reply);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "send message failed");
        return RES_FAILURE;
    }
    return RES_SUCCESS;
}

int32_t ScreenSessionAbilityConnection::SendMessageBlock(
    const int32_t &transCode, MessageParcel &data, MessageParcel &reply)
{
    if (abilityConnectionStub_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "ability connection is nullptr");
        return RES_FAILURE;
    }
    int32_t ret = abilityConnectionStub_->SendMessageSyncBlock(transCode, data, reply);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "send message failed");
        return RES_FAILURE;
    }
    return RES_SUCCESS;
}

int32_t ScreenSessionAbilityConnectionStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    [[maybe_unused]]MessageParcel& reply, [[maybe_unused]]MessageOption& option)
{
    TLOGI(WmsLogTag::DMS, "OnRemoteRequest code is %{public}u", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TLOGE(WmsLogTag::DMS, "InterfaceToken check failed");
        return ERR_INVALID_DATA;
    }
    uint32_t msgId = code;
    switch (msgId) {
        case TRANS_RELEASE_BLOCK: {
            screenId_ = data.ReadInt32();
            left_ = data.ReadInt32();
            top_ = data.ReadInt32();
            width_ = data.ReadInt32();
            height_ = data.ReadInt32();
            break;
        }
        case TRANS_FAILED_FOR_PRIVACY: {
            errCode_ = msgId;
            break;
        }
        default:
            TLOGI(WmsLogTag::DMS, "unknown transaction code");
    }
    sendMessageWaitFlag_ = true;
    blockSendMessageCV_.notify_all();
    TLOGI(WmsLogTag::DMS, "blockSendMessageCV_ notify");
    return msgId;
}

int32_t ScreenSessionAbilityConnectionStub::GetScreenId()
{
    return screenId_;
}

int32_t ScreenSessionAbilityConnectionStub::GetLeft()
{
    return left_;
}

int32_t ScreenSessionAbilityConnectionStub::GetTop()
{
    return top_;
}

int32_t ScreenSessionAbilityConnectionStub::GetWidth()
{
    return width_;
}

int32_t ScreenSessionAbilityConnectionStub::GetHeight()
{
    return height_;
}

int32_t ScreenSessionAbilityConnectionStub::GetErrCode()
{
    return errCode_;
}

void ScreenSessionAbilityConnectionStub::EraseErrCode()
{
    errCode_ = 0;
}

bool ScreenSessionAbilityConnection::IsConnected()
{
    if (abilityConnectionStub_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "ability connect failed");
        return false;
    }
    return abilityConnectionStub_->IsAbilityConnected();
}

bool ScreenSessionAbilityConnection::IsConnectedSync()
{
    if (abilityConnectionStub_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "ability connect failed");
        return false;
    }
    return abilityConnectionStub_->IsAbilityConnectedSync();
}

sptr<ScreenSessionAbilityConnectionStub> ScreenSessionAbilityConnection::GetScreenSessionAbilityConnectionStub()
{
    return abilityConnectionStub_;
}

} // namespace OHOS::Rosen