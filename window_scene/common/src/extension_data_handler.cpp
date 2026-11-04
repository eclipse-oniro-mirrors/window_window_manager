/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "common/include/extension_data_handler.h"

#include <sstream>

#include <hitrace_meter.h>
#include <iremote_proxy.h>
#include <message_parcel.h>
#include <securec.h>
#include <want.h>

#include "window_manager_hilog.h"

namespace OHOS::Rosen::Extension {

bool DataTransferConfig::Marshalling(Parcel& parcel) const
{
    return parcel.WriteUint8(static_cast<uint8_t>(subSystemId)) && parcel.WriteUint32(customId) &&
           parcel.WriteBool(needReply) && parcel.WriteBool(needSyncSend);
}

DataTransferConfig* DataTransferConfig::Unmarshalling(Parcel& parcel)
{
    auto config = new DataTransferConfig();
    uint8_t subSystemIdValue = static_cast<uint8_t>(SubSystemId::INVALID);
    if (!parcel.ReadUint8(subSystemIdValue) || subSystemIdValue >= static_cast<uint8_t>(SubSystemId::INVALID)) {
        delete config;
        return nullptr;
    }
    if (!parcel.ReadUint32(config->customId) || !parcel.ReadBool(config->needReply) ||
        !parcel.ReadBool(config->needSyncSend)) {
        delete config;
        return nullptr;
    }
    config->subSystemId = static_cast<SubSystemId>(subSystemIdValue);
    return config;
}

std::string DataTransferConfig::ToString() const
{
    std::string str;
    constexpr int BUFFER_SIZE = 128;
    char buffer[BUFFER_SIZE] = { 0 };
    if (snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1,
                   "subSystemId: %hhu, customId: %u, needReply: %d, needSyncSend: %d", subSystemId, customId, needReply,
                   needSyncSend) > 0) {
        str.append(buffer);
    }
    return str;
}

DataHandlerErr DataHandler::RegisterDataConsumer(SubSystemId subSystemId, DataConsumeCallback&& callback)
{
    std::lock_guard lock(mutex_);
    if (consumers_.find(subSystemId) != consumers_.end()) {
        // A consumer already exists for this subSystemId
        TLOGE(WmsLogTag::WMS_UIEXT, "Consumer already exists for subSystemId: %{public}hhu", subSystemId);
        return DataHandlerErr::DUPLICATE_REGISTRATION;
    }
    consumers_.emplace(subSystemId, std::move(callback));
    TLOGI(WmsLogTag::WMS_UIEXT, "Register consumer: %{public}hhu", subSystemId);
    return DataHandlerErr::OK;
}

void DataHandler::UnregisterDataConsumer(SubSystemId subSystemId)
{
    std::lock_guard lock(mutex_);
    consumers_.erase(subSystemId);
    TLOGI(WmsLogTag::WMS_UIEXT, "Unregister consumer: %{public}hhu", subSystemId);
}

bool DataHandler::IsProxyObject() const
{
    std::lock_guard lock(mutex_);
    return remoteProxy_ && remoteProxy_->IsProxyObject();
}

DataHandlerErr DataHandler::PrepareSendData(MessageParcel& data, const DataTransferConfig& config,
                                            const AAFwk::Want& toSend)
{
    if (IsProxyObject() && !WriteInterfaceToken(data)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write interface token failed, %{public}s", config.ToString().c_str());
        return DataHandlerErr::WRITE_PARCEL_ERROR;
    }

    if (!data.WriteParcelable(&config)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write config failed, %{public}s", config.ToString().c_str());
        return DataHandlerErr::WRITE_PARCEL_ERROR;
    }

    if (!data.WriteParcelable(&toSend)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write toSend failed, %{public}s", config.ToString().c_str());
        return DataHandlerErr::WRITE_PARCEL_ERROR;
    }
    return DataHandlerErr::OK;
}

DataHandlerErr DataHandler::ParseReply(MessageParcel& replyParcel, AAFwk::Want& reply, const DataTransferConfig& config)
{
    if (!config.needReply) {
        return DataHandlerErr::OK;
    }

    uint32_t replyCode = 0;
    if (!replyParcel.ReadUint32(replyCode)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read replyCode failed, %{public}s", config.ToString().c_str());
        return DataHandlerErr::READ_PARCEL_ERROR;
    }

    if (config.needReply) {
        sptr<AAFwk::Want> response = replyParcel.ReadParcelable<AAFwk::Want>();
        if (!response) {
            TLOGE(WmsLogTag::WMS_UIEXT, "read response failed, %{public}s", config.ToString().c_str());
            return DataHandlerErr::READ_PARCEL_ERROR;
        }
        reply = std::move(*response);
    }

    return static_cast<DataHandlerErr>(replyCode);
}

// process data from peer
void DataHandler::NotifyDataConsumer(MessageParcel& recieved, MessageParcel& reply)
{
    sptr<DataTransferConfig> config = recieved.ReadParcelable<DataTransferConfig>();
    if (config == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read config failed");
        reply.WriteUint32(static_cast<uint32_t>(DataHandlerErr::READ_PARCEL_ERROR));
        return;
    }

    sptr<AAFwk::Want> sendWant = recieved.ReadParcelable<AAFwk::Want>();
    if (sendWant == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read want failed");
        reply.WriteUint32(static_cast<uint32_t>(DataHandlerErr::READ_PARCEL_ERROR));
        return;
    }

    std::optional<AAFwk::Want> replyWant;
    bool needReply = (config->needSyncSend && config->needReply);
    if (needReply) {
        replyWant = std::make_optional<AAFwk::Want>();
    }

    auto ret = NotifyDataConsumer(std::move(*sendWant), replyWant, *config);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    if (needReply && replyWant) {
        reply.WriteParcelable(&(replyWant.value()));
    }
}

void DataHandler::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler>& eventHandler)
{
    eventHandler_ = eventHandler;
}

void DataHandler::SetRemoteProxyObject(const sptr<IRemoteObject>& remoteObject)
{
    std::lock_guard lock(mutex_);
    remoteProxy_ = remoteObject;
}

DataHandlerErr DataHandler::SendDataSync(SubSystemId subSystemId, uint32_t customId, const AAFwk::Want& toSend,
                                         AAFwk::Want& reply)
{
    DataTransferConfig config;
    config.needSyncSend = true;
    config.needReply = true;
    config.subSystemId = subSystemId;
    config.customId = customId;
    return SendData(toSend, reply, config);
}

DataHandlerErr DataHandler::SendDataSync(SubSystemId subSystemId, uint32_t customId, const AAFwk::Want& toSend)
{
    DataTransferConfig config;
    config.needSyncSend = true;
    config.subSystemId = subSystemId;
    config.customId = customId;
    AAFwk::Want reply;
    return SendData(toSend, reply, config);
}

DataHandlerErr DataHandler::SendDataAsync(SubSystemId subSystemId, uint32_t customId, const AAFwk::Want& toSend)
{
    DataTransferConfig config;
    config.subSystemId = subSystemId;
    config.customId = customId;
    AAFwk::Want reply;
    return SendData(toSend, reply, config);
}

DataHandlerErr DataHandler::NotifyDataConsumer(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply,
                                               const DataTransferConfig& config)
{
    DataConsumeCallback callback;
    {
        std::lock_guard lock(mutex_);
        auto it = consumers_.find(config.subSystemId);
        if (it == consumers_.end()) {
            TLOGE(WmsLogTag::WMS_UIEXT, "not found, %{public}s", config.ToString().c_str());
            return DataHandlerErr::NO_CONSUME_CALLBACK;
        }
        callback = it->second;
    }

    if (!callback) {
        TLOGE(WmsLogTag::WMS_UIEXT, "not callable, %{public}s", config.ToString().c_str());
        return DataHandlerErr::INVALID_CALLBACK;
    }

    // sync mode
    if (config.needSyncSend) {
        auto ret = callback(config.subSystemId, config.customId, std::move(data), reply);
        TLOGD(WmsLogTag::WMS_UIEXT, "subSystemId: %{public}hhu, customId: %{public}u, ret: %{public}d",
              config.subSystemId, config.customId, ret);
        return DataHandlerErr::OK;
    }

    // async mode
    auto task = [input = std::move(data), subSystemId = config.subSystemId, customId = config.customId,
                 func = std::move(callback)]() mutable {
        std::optional<AAFwk::Want> reply;
        auto ret = func(subSystemId, customId, std::move(input), reply);
        TLOGND(WmsLogTag::WMS_UIEXT, "subSystemId: %{public}hhu, customId: %{public}u, ret: %{public}d", subSystemId,
               customId, ret);
    };

    std::ostringstream oss;
    oss << "NotifyDataConsumer_" << static_cast<uint32_t>(config.subSystemId) << "_" << config.customId;
    PostAsyncTask(std::move(task), oss.str(), 0);
    return DataHandlerErr::OK;
}

void DataHandler::PostAsyncTask(Task&& task, const std::string& name, int64_t delayTime)
{
    if (!eventHandler_) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "uiext:%s", name.c_str());
        task();
        return;
    }

    if (auto runner = eventHandler_->GetEventRunner(); !runner || runner->IsCurrentRunnerThread()) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "uiext:%s", name.c_str());
        task();
        return;
    }

    auto localTask = [task = std::move(task), name] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "uiext:%s", name.c_str());
        task();
    };

    auto ret = eventHandler_->PostTask(std::move(localTask), "uiext:" + name, delayTime,
                                       AppExecFwk::EventQueue::Priority::IMMEDIATE);
    if (!ret) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Post task failed, name: %{public}s", name.c_str());
    }
}
}  // namespace OHOS::Rosen::Extension