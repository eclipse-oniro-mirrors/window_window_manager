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

#include "session_manager/include/zidl/pip_change_listener_proxy.h"
#include "hilog_tag_wrapper.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
void PipChangeListenerProxy::OnPipStart(int32_t windowId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "Write interfaceToken failed");
        return;
    }

    if (!data.WriteInt32(windowId)) {
        TLOGE(WmsLogTag::WMS_PIP, "Write windowId failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return;
    }
    int errCode = remote->SendRequest(static_cast<uint32_t>(
        IPipChangeListenerMessage::TRANS_ON_PIP_CHANGE_EVENT), data, reply, option);
    if (errCode != NO_ERROR) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed %{public}d.", errCode);
        return;
    }
}
} // namespace OHOS::Rosen