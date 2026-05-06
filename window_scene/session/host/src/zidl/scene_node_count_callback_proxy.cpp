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

#include "zidl/scene_node_count_callback_proxy.h"
#include <ipc_types.h>
#include <message_parcel.h>
#include <message_option.h>
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

void SceneNodeCountCallbackProxy::OnSceneNodeCount(uint32_t nodeCount)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ROTATION, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(nodeCount)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Write nodeCount failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "remote is null");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneNodeCountCallbackMessage::TRANS_ON_SCENE_NODE_COUNT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "SendRequest failed");
    }
}
} // namespace Rosen
} // namespace OHOS
