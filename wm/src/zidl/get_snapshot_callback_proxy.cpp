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
#include "zidl/get_snapshot_callback_proxy.h"
#include "window_manager_hilog.h"
#include "pixel_map.h"
 
namespace OHOS::Rosen {
void GetSnapshotCallbackProxy::OnReceived(WMError errCode,
    const std::vector<std::shared_ptr<OHOS::Media::PixelMap>>& pixelMaps)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
 
    if (!data.WriteInterfaceToken(IGetSnapshotCallback::GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interface token failed.");
        return;
    }
    if (!data.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LIFE, "write error code failed");
        return;
    }
    if (!WritePixelMapData(data, pixelMaps)) {
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        IGetSnapshotCallbackMessage::TRANS_ON_GET_SNAPSHOT), data, reply, option) != NO_ERROR) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed.");
        return;
    }
}

bool GetSnapshotCallbackProxy::WritePixelMapData(MessageParcel& data,
    const std::vector<std::shared_ptr<OHOS::Media::PixelMap>>& pixelMaps)
{
    std::vector<int32_t> nullptrVectors;
    for (size_t i = 0; i < pixelMaps.size(); i++) {
        if (pixelMaps[i] == nullptr) {
            nullptrVectors.push_back(static_cast<int32_t>(i));
        }
    }
    int32_t len = static_cast<int32_t>(pixelMaps.size() - nullptrVectors.size());
    if (!data.WriteInt32(len)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write size failed");
        return false;
    }
    for (size_t i = 0; i < pixelMaps.size(); i++) {
        if (pixelMaps[i] == nullptr) {
            continue;
        }
        if (!pixelMaps[i]->Marshalling(data)) {
            TLOGE(WmsLogTag::WMS_LIFE, "write pixelMap failed");
            return false;
        }
    }
    if (nullptrVectors.size() > 0) {
        if (!data.WriteInt32(static_cast<int32_t>(nullptrVectors.size()))) {
            TLOGE(WmsLogTag::WMS_LIFE, "write nullptrVectors size failed");
            return false;
        }
        for (size_t i = 0; i < nullptrVectors.size(); i++) {
            if (!data.WriteInt32(nullptrVectors[i])) {
                TLOGE(WmsLogTag::WMS_LIFE, "write nullptrVectors failed");
                return false;
            }
        }
    } else {
        if (!data.WriteInt32(0)) {
            TLOGE(WmsLogTag::WMS_LIFE, "write nullptrVectors size zero failed");
            return false;
        }
    }
    return true;
}
}