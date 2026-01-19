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
#include "zidl/get_snapshot_callback_stub.h"
#include "window_manager_hilog.h"
#include "pixel_map.h"
 
namespace OHOS::Rosen {
const int32_t MAX_SESSION_LIMIT_ALL_APP = 512;
 
int GetSnapshotCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TLOGE(WmsLogTag::WMS_LIFE, "local descriptor not equal to remote.");
        return ERR_INVALID_STATE;
    }
    return HandleOnReceived(data, reply);
}
 
int GetSnapshotCallbackStub::HandleOnReceived(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    std::vector<std::shared_ptr<OHOS::Media::PixelMap>> pixelMaps;
    int32_t errCode = 0;
    int32_t len = 0;
    int32_t nullptrLen = 0;
    std::vector<int32_t> nullptrVectors;
    if (!data.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read errcode failed");
        OnReceived(WMError::WM_ERROR_SYSTEM_ABNORMALLY, pixelMaps);
        return ERR_NONE;
    }
    if (!data.ReadInt32(len)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read len failed");
        OnReceived(static_cast<WMError>(errCode), pixelMaps);
        return ERR_NONE;
    }
    if (len <= 0) {
        OnReceived(static_cast<WMError>(errCode), pixelMaps);
        return ERR_NONE;
    }
    size_t size = static_cast<size_t>(len);
    pixelMaps.resize(size);
    for (size_t i = 0; i < size; i++) {
        pixelMaps[i] = std::shared_ptr<OHOS::Media::PixelMap>(OHOS::Media::PixelMap::Unmarshalling(data));
    }
    if (!data.ReadInt32(nullptrLen)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read nullptrLen failed");
    }
    if (nullptrLen > 0 && nullptrLen <= MAX_SESSION_LIMIT_ALL_APP) {
        int32_t index = 0;
        for (size_t i = 0; i < static_cast<size_t>(nullptrLen); i++) {
            if (!data.ReadInt32(index)) {
                OnReceived(static_cast<WMError>(errCode), pixelMaps);
                return ERR_NONE;
            }
            if (index > static_cast<int32_t>(pixelMaps.size()) || index < 0) {
                OnReceived(static_cast<WMError>(errCode), pixelMaps);
                return ERR_NONE;
            }
            pixelMaps.insert(pixelMaps.begin() + index, nullptr);
        }
    }
    TLOGI(WmsLogTag::WMS_LIFE, "pixelMaps size: %{public}zu, size: %{public}zu", pixelMaps.size(), size);
    OnReceived(static_cast<WMError>(errCode), pixelMaps);
    return ERR_NONE;
}
}