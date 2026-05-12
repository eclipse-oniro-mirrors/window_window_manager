/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "zidl/scene_node_count_callback_stub.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

int SceneNodeCountCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "OnRemoteRequest code is %{public}u", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TLOGE(WmsLogTag::WMS_ROTATION, "InterfaceToken check failed");
        return ERR_TRANSACTION_FAILED;
    }
    SceneNodeCountCallbackMessage msgId = static_cast<SceneNodeCountCallbackMessage>(code);
    switch (msgId) {
        case SceneNodeCountCallbackMessage::TRANS_ON_SCENE_NODE_COUNT: {
            uint32_t nodeCount = 0;
            if (!data.ReadUint32(nodeCount)) {
                TLOGE(WmsLogTag::WMS_ROTATION, "Read nodeCount failed.");
                return ERR_INVALID_DATA;
            }
            OnSceneNodeCount(nodeCount);
            break;
        }
        default:
            TLOGW(WmsLogTag::WMS_ROTATION, "Unknown transaction code %{public}u", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_NONE;
}
} // namespace Rosen
} // namespace OHOS