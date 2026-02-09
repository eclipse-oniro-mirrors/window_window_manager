/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_SCREEN_SESSION_ABILITY_CONNECTION_H
#define OHOS_ROSEN_SCREEN_SESSION_ABILITY_CONNECTION_H

#include <string>
#include <mutex>
#include <condition_variable>

#include "ability_connection.h"
#include "iremote_object.h"

namespace OHOS::Rosen {
class ScreenSessionAbilityDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit ScreenSessionAbilityDeathRecipient(
        std::function<void(void)> deathHandler) : deathHandler_(deathHandler) {}
    ~ScreenSessionAbilityDeathRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remoteObject) override;

private:
    std::function<void(void)> deathHandler_;
};

class ScreenSessionAbilityConnectionStub : public AAFwk::AbilityConnectionStub {
public:
    explicit ScreenSessionAbilityConnectionStub() = default;
    virtual ~ScreenSessionAbilityConnectionStub() = default;

    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override;
    int32_t SendMessageSync(int32_t transCode, MessageParcel &data, MessageParcel &reply);
    int32_t SendMessageSyncBlock(int32_t transCode, MessageParcel &data, MessageParcel &reply);
    bool IsAbilityConnected();
    bool IsAbilityConnectedSync();
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    int32_t GetScreenId();
    int32_t GetLeft();
    int32_t GetTop();
    int32_t GetWidth();
    int32_t GetHeight();
    int32_t GetErrCode();
    void EraseErrCode();

private:
    bool AddObjectDeathRecipient();

private:
    std::atomic<bool> isConnected_{false};
    sptr<IRemoteObject> remoteObject_;
    std::mutex remoteObjectMutex_;
    sptr<ScreenSessionAbilityDeathRecipient> deathRecipient_;
    std::mutex connectedMutex_;
    std::condition_variable connectedCv_;
    std::mutex sendMessageMutex_;
    std::condition_variable blockSendMessageCV_;
    std::atomic<bool> sendMessageWaitFlag_{false};
    int32_t screenId_ = 0;
    int32_t left_ = 0;
    int32_t top_ = 0;
    int32_t width_ = 0;
    int32_t height_ = 0;
    uint32_t errCode_ = 0;
};

class ScreenSessionAbilityConnection {
public:
    explicit ScreenSessionAbilityConnection() = default;
    ~ScreenSessionAbilityConnection() = default;

    bool ScreenSessionConnectExtension(const std::string &bundleName, const std::string &abilityName);
    bool ScreenSessionConnectExtension(const std::string &bundleName, const std::string &abilityName,
        const std::vector<std::pair<std::string, std::string>> &params);
    void ScreenSessionDisconnectExtension();
    int32_t SendMessage(const int32_t &transCode, MessageParcel &data, MessageParcel &reply);
    int32_t SendMessageBlock(const int32_t &transCode, MessageParcel &data, MessageParcel &reply);
    bool IsConnected();
    bool IsConnectedSync();
    sptr<ScreenSessionAbilityConnectionStub> GetScreenSessionAbilityConnectionStub();

private:
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub_;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_SCREEN_SESSION_ABILITY_CONNECTION_H