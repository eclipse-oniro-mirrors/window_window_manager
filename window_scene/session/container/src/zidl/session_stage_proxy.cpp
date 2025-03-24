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

#include "session/container/include/zidl/session_stage_proxy.h"
#include "session/container/include/zidl/session_stage_ipc_interface_code.h"

#include <cstdint>
#include <ipc_types.h>
#include <message_option.h>
#include <message_parcel.h>
#include <securec.h>

#include "window_manager_hilog.h"
#include "ws_common.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "SessionStageProxy"};
constexpr int32_t MAX_INFO_SIZE = 50;
constexpr size_t MAX_PARCEL_CAPACITY = 100 * 1024 * 1024; // 100M

bool CopyBufferFromRawData(void*& buffer, size_t size, const void* data)
{
    if (data == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "data is nullptr");
        return false;
    }

    if (size == 0 || size >= MAX_PARCEL_CAPACITY) {
        TLOGE(WmsLogTag::WMS_UIEXT, "size is invalid");
        return false;
    }

    buffer = malloc(size);
    if (buffer == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "buffer malloc failed");
        return false;
    }

    if (memcpy_s(buffer, size, data, size) != EOK) {
        free(buffer);
        TLOGE(WmsLogTag::WMS_UIEXT, "memcpy_s failed");
        return false;
    }

    return true;
}

bool ReadLittleStringVectorFromParcel(MessageParcel& reply, std::vector<std::string>& infos)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "entry");
    if (!reply.ReadStringVector(&infos)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Read string vector failed");
        return false;
    }
    return true;
}

bool ReadLargeStringVectorFromParcel(MessageParcel& reply, std::vector<std::string>& infos)
{
    int32_t dataSizeInt = 0;
    if (!reply.ReadInt32(dataSizeInt) || dataSizeInt == 0) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Read dataSize failed");
        return false;
    }

    size_t dataSize = static_cast<size_t>(dataSizeInt);
    void* buffer = nullptr;
    if (!CopyBufferFromRawData(buffer, dataSize, reply.ReadRawData(dataSize))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Read rawData failed, dataSize: %{public}zu", dataSize);
        return false;
    }

    MessageParcel readParcel;
    if (!readParcel.ParseFrom(reinterpret_cast<uintptr_t>(buffer), dataSize)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Parse from buffer failed");
        return false;
    }

    int32_t infoSize = 0;
    if (!readParcel.ReadInt32(infoSize)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Read infoSize failed");
        return false;
    }

    TLOGD(WmsLogTag::WMS_UIEXT, "dataSize: %{public}zu, infoSize: %{public}d", dataSize, infoSize);
    if (infoSize >= MAX_INFO_SIZE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Too big infos, infoSize: %{public}d", infoSize);
        return false;
    }

    infos.clear();
    infos.reserve(infoSize);
    for (int32_t i = 0; i < infoSize; i++) {
        infos.emplace_back(readParcel.ReadString());
    }

    return true;
}
}

WSError SessionStageProxy::SetActive(bool active)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(active)) {
        WLOGFE("Write active failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_ACTIVE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionStageProxy::UpdateDisplayId(uint64_t displayId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint64(displayId)) {
        WLOGFE("Write displayId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DISPLAYID_CHANGE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionStageProxy::UpdateRect(const WSRect& rect, SizeChangeReason reason,
    const SceneAnimationConfig& config, const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!(data.WriteInt32(rect.posX_) && data.WriteInt32(rect.posY_) &&
        data.WriteUint32(rect.width_) && data.WriteUint32(rect.height_))) {
        WLOGFE("Write WindowRect failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(reason))) {
        WLOGFE("Write SessionSizeChangeReason failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    const std::shared_ptr<RSTransaction>& rsTransaction = config.rsTransaction_;
    bool hasRSTransaction = rsTransaction != nullptr;
    if (!data.WriteBool(hasRSTransaction)) {
        WLOGFE("Write has transaction failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (hasRSTransaction) {
        auto pid = rsTransaction->GetParentPid();
        rsTransaction->SetParentPid(getprocpid());
        if (!data.WriteParcelable(rsTransaction.get())) {
            WLOGFE("Write transaction sync Id failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
        rsTransaction->SetParentPid(pid);
    }

    if (!data.WriteInt32(config.animationDuration_)) {
        TLOGE(WmsLogTag::DEFAULT, "Write animation duration failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(avoidAreas.size())) {
        TLOGE(WmsLogTag::WMS_IMMS, "Write avoid area size failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    for (const auto& [type, avoidArea] : avoidAreas) {
        if (!data.WriteUint32(static_cast<uint32_t>(type))) {
            TLOGE(WmsLogTag::WMS_IMMS, "Write avoid area type failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
        if (!data.WriteParcelable(&avoidArea)) {
            TLOGE(WmsLogTag::WMS_IMMS, "Write avoid area failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SIZE_CHANGE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

void SessionStageProxy::UpdateDensity()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DENSITY_CHANGE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return;
    }
}

WSError SessionStageProxy::UpdateOrientation()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_ORIENTATION_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    WSError ret = static_cast<WSError>(reply.ReadInt32());
    if (ret != WSError::WS_OK) {
        TLOGE(WmsLogTag::DMS, "update orientation by ipc failed with error: %{public}d.", ret);
    }
    return ret;
}

WSError SessionStageProxy::HandleBackEvent()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_HANDLE_BACK_EVENT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionStageProxy::SwitchFreeMultiWindow(bool enable)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DEFAULT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(enable)) {
        TLOGE(WmsLogTag::DEFAULT, "Write enable failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SWITCH_FREEMULTIWINDOW),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DEFAULT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionStageProxy::GetUIContentRemoteObj(sptr<IRemoteObject>& uiContentRemoteObj)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DEFAULT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_GET_UI_CONTENT_REMOTE_OBJ),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DEFAULT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remoteObj = reply.ReadRemoteObject();
    if (remoteObj == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "ReadRemoteObject failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    uiContentRemoteObj = remoteObj;
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SessionStageProxy::MarkProcessed(int32_t eventId)
{
    return WSError::WS_DO_NOTHING;
}

WSError SessionStageProxy::NotifyDestroy()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DESTROY),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionStageProxy::NotifyCloseExistPipWindow()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_CLOSE_EXIST_PIP_WINDOW),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionStageProxy::UpdateFocus(bool focus)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(focus)) {
        WLOGFE("Write focus failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_FOCUS_CHANGE),
        data, reply, option) != ERR_NONE) {
        WLOGFW("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionStageProxy::NotifyTransferComponentData(const AAFwk::WantParams& wantParams)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteParcelable(&wantParams)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "wantParams write failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TRANSFER_COMPONENT_DATA),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, code: %{public}d", sendCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSErrorCode SessionStageProxy::NotifyTransferComponentDataSync(const AAFwk::WantParams& wantParams,
                                                               AAFwk::WantParams& reWantParams)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return WSErrorCode::WS_ERROR_TRANSFER_DATA_FAILED;
    }

    if (!data.WriteParcelable(&wantParams)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "wantParams write failed.");
        return WSErrorCode::WS_ERROR_TRANSFER_DATA_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return WSErrorCode::WS_ERROR_TRANSFER_DATA_FAILED;
    }

    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TRANSFER_COMPONENT_DATA_SYNC),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, code: %{public}d", sendCode);
        return static_cast<WSErrorCode>(sendCode);
    }

    std::shared_ptr<AAFwk::WantParams> readWantParams(reply.ReadParcelable<AAFwk::WantParams>());
    if (readWantParams == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "readWantParams is nullptr");
        return WSErrorCode::WS_ERROR_TRANSFER_DATA_FAILED;
    }

    reWantParams = *readWantParams;
    return WSErrorCode::WS_OK;
}

void SessionStageProxy::NotifyOccupiedAreaChangeInfo(sptr<OccupiedAreaChangeInfo> info,
                                                     const std::shared_ptr<RSTransaction>& rsTransaction)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteParcelable(info.GetRefPtr())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "occupied info write failed.");
        return;
    }

    bool hasRSTransaction = rsTransaction != nullptr;
    if (!data.WriteBool(hasRSTransaction)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Write has transaction failed");
        return;
    }
    if (hasRSTransaction) {
        if (!data.WriteParcelable(rsTransaction.get())) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Write transaction sync Id failed");
            return;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "remote is null");
        return;
    }

    if (remote->SendRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_OCCUPIED_AREA_CHANGE_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest failed");
        return;
    }
    return;
}

WSError SessionStageProxy::UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_IMMS, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteStrongParcelable(avoidArea)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Write AvoidArea failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        TLOGE(WmsLogTag::WMS_IMMS, "Write AvoidAreaType failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_AVOID_AREA), data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_IMMS, "SendRequest failed, code: %{public}d", sendCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

void SessionStageProxy::DumpSessionElementInfo(const std::vector<std::string>& params)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteStringVector(params)) {
        WLOGFE("Write params failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_DUMP_SESSSION_ELEMENT_INFO),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return;
    }
}

void SessionStageProxy::NotifyScreenshot()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SCREEN_SHOT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return;
    }
}

WSError SessionStageProxy::NotifyTouchOutside()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TOUCH_OUTSIDE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionStageProxy::NotifyWindowVisibility(bool isVisible)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(isVisible)) {
        WLOGFE("Write window visible failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    uint32_t messageCode = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_WINDOW_VISIBILITY_CHANGE);
    
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(messageCode, data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionStageProxy::UpdateWindowMode(WindowMode mode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(mode))) {
        WLOGFE("Write mode failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_WINDOW_MODE_CHANGE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

void SessionStageProxy::NotifyForegroundInteractiveStatus(bool interactive)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteBool(interactive)) {
        WLOGFE("Write interactive failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_FOREGROUND_INTERACTIVE_STATUS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

WSError SessionStageProxy::UpdateMaximizeMode(MaximizeMode mode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("UpdateMaximizeMode WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(mode))) {
        WLOGFE("UpdateMaximizeMode Write mode failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_MAXIMIZE_MODE_CHANGE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("UpdateMaximizeMode SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

void  SessionStageProxy::NotifySessionForeground(uint32_t reason, bool withAnimation)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteUint32(reason)) {
        WLOGFE("Write reason failed");
        return;
    }
    if (!data.WriteBool(withAnimation)) {
        WLOGFE("Write withAnimation failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SESSION_FOREGROUND),
        data, reply, option) != ERR_NONE) {
        WLOGFE("Send NotifySessionForeground Request failed");
    }
}

void SessionStageProxy::NotifySessionFullScreen(bool fullScreen)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteBool(fullScreen)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write fullScreen failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SESSION_FULLSCREEN),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Send Request failed");
    }
}

void SessionStageProxy::NotifySessionBackground(uint32_t reason, bool withAnimation, bool isFromInnerkits)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteUint32(reason)) {
        WLOGFE("Write reason failed");
        return;
    }
    if (!data.WriteBool(withAnimation)) {
        WLOGFE("Write withAnimation failed");
        return;
    }
    if (!data.WriteBool(isFromInnerkits)) {
        WLOGFE("Write isFromInnerkits failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SESSION_BACKGROUND),
        data, reply, option) != ERR_NONE) {
        WLOGFE("Send NotifySessionBackground Request failed");
        return;
    }
}

WSError SessionStageProxy::UpdateTitleInTargetPos(bool isShow, int32_t height)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(isShow)) {
        WLOGFE("Write isShow failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(height)) {
        WLOGFE("Write height failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TITLE_POSITION_CHANGE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

void SessionStageProxy::NotifyTransformChange(const Transform& transform)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    if (!transform.Marshalling(data)) {
        WLOGFE("Transform marshalling failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TRANSFORM_CHANGE),
                            data, reply, option) != ERR_NONE) {
        WLOGFE("Send NotifyTransformChange Requset failed");
    }
}

void SessionStageProxy::NotifySingleHandTransformChange(const SingleHandTransform& singleHandTransform)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return;
    }

    if (!singleHandTransform.Marshalling(data)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "singleHandTransform marshalling failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SINGLE_HAND_TRANSFORM),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Send Requset failed");
    }
}

WSError SessionStageProxy::NotifyDensityFollowHost(bool isFollowHost, float densityValue)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(isFollowHost)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write isFollowHost failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteFloat(densityValue)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write densityValue failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DENSITY_FOLLOW_HOST), data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, code: %{public}d", sendCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }

    return WSError::WS_OK;
}

WSError SessionStageProxy::NotifyDialogStateChange(bool isForeground)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(isForeground)) {
        WLOGFE("Write isForeground failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DIALOG_STATE_CHANGE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionStageProxy::SetPipActionEvent(const std::string& action, int32_t status)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteString(action)) {
        WLOGFE("Write params failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteInt32(status)) {
        WLOGFE("Write status failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_PIP_ACTION_EVENT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionStageProxy::NotifyPipWindowSizeChange(uint32_t width, uint32_t height, double scale)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(width)) {
        WLOGFE("Write width failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(height)) {
        WLOGFE("Write height failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteFloat(scale)) {
        WLOGFE("Write scale failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_PIPSIZE_CHANGE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionStageProxy::SetPiPControlEvent(WsPiPControlType controlType, WsPiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType:%{public}u, enabled:%{public}d", controlType, status);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(controlType))) {
        TLOGE(WmsLogTag::WMS_PIP, "Write params failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteInt32(static_cast<int32_t>(status))) {
        TLOGE(WmsLogTag::WMS_PIP, "Write status failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_PIP_CONTROL_EVENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

void SessionStageProxy::NotifyDisplayMove(DisplayId from, DisplayId to)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteUint64(from)) {
        WLOGFE("Write from id failed");
        return;
    }

    if (!data.WriteUint64(to)) {
        WLOGFE("Write to id failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DISPLAY_MOVE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest notify display move failed");
        return;
    }
}

void SessionStageProxy::NotifyKeyboardPanelInfoChange(const KeyboardPanelInfo& keyboardPanelInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteParcelable(&keyboardPanelInfo)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "KeyboardPanelInfo marshalling failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_KEYBOARD_INFO_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest notify keyboard panel info change failed");
        return;
    }
}

WSError SessionStageProxy::NotifyCompatibleModeEnableInPad(bool enable)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SCB, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(enable)) {
        TLOGE(WmsLogTag::DEFAULT, "Write enable failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_COMPATIBLE_MODE_ENABLE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SCB, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

void SessionStageProxy::SetUniqueVirtualPixelRatio(bool useUniqueDensity, float virtualPixelRatio)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DEFAULT, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteBool(useUniqueDensity)) {
        TLOGE(WmsLogTag::DEFAULT, "Write useUniqueDensity failed");
        return;
    }
    if (!data.WriteFloat(virtualPixelRatio)) {
        TLOGE(WmsLogTag::DEFAULT, "Write virtualPixelRatio failed");
        return;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DENSITY_UNIQUE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DEFAULT, "SendRequest failed");
        return;
    }
}

WSError SessionStageProxy::NotifyDumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteStringVector(params)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write params failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DUMP_INFO),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, code: %{public}d", sendCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }

    bool isLittleSize = false;
    if (!reply.ReadBool(isLittleSize)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "ReadBool failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    bool readResult = isLittleSize ? ReadLittleStringVectorFromParcel(reply, info) :
        ReadLargeStringVectorFromParcel(reply, info);
    if (!readResult) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Read data failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Read int32 failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WSError SessionStageProxy::SendExtensionData(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }

    auto ret = remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SEND_EXTENSION_DATA), data,
                                   reply, option);
    if (ret != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, ret: %{public}d", ret);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionStageProxy::SetDragActivated(bool dragActivated)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(dragActivated)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write params failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_DRAG_ACTIVATED),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionStageProxy::SetEnableDragBySystem(bool dragEnable)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(dragEnable)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write params failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_ENABLE_DRAG_BY_SYSTEM),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionStageProxy::NotifyHighlightChange(bool isHighlight)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(isHighlight)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write isHighlight failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_HIGHLIGHT_CHANGE),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_FOCUS, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionStageProxy::NotifyWindowAttachStateChange(bool isAttach)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SUB, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isAttach)) {
        TLOGE(WmsLogTag::WMS_SUB, "Write params failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_WINDOW_ATTACH_STATE_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SUB, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionStageProxy::SetCurrentRotation(int32_t currentRotation)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ROTATION, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteInt32(currentRotation)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Write params failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_CURRENT_ROTATION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}
} // namespace OHOS::Rosen
