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

#ifndef OHOS_ROSEN_SCREEN_SESSION_MANAGER_STUB_H
#define OHOS_ROSEN_SCREEN_SESSION_MANAGER_STUB_H

#include "dms_global_mutex.h"
#include "screen_session_manager_interface.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Rosen {

class ScreenSessionManagerStub : public IRemoteStub<IScreenSessionManager> {
public:
    ScreenSessionManagerStub() = default;

    ~ScreenSessionManagerStub() = default;

    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;
    virtual int32_t OnRemoteRequestInner(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& option);
private:
    void ProcGetSnapshotByPicker(MessageParcel& reply);
    void ProcSetVirtualScreenFlag(MessageParcel& data, MessageParcel& reply);
    void ProcGetVirtualScreenFlag(MessageParcel& data, MessageParcel& reply);
    void ProcGetAvailableArea(MessageParcel& data, MessageParcel& reply);
    void ProcProxyForFreeze(MessageParcel& data, MessageParcel& reply);
    void ProcGetAllDisplayPhysicalResolution(MessageParcel& data, MessageParcel& reply);
    void ProcSetVirtualScreenSecurityExemption(MessageParcel& data, MessageParcel& reply);
    void ProcGetScreenCapture(MessageParcel& data, MessageParcel& reply);
    void ProcGetDisplaySnapshotWithOption(MessageParcel& data, MessageParcel& reply);
    void ProcGetDisplayHDRSnapshotWithOption(MessageParcel& data, MessageParcel& reply);
    void ProcSetScreenSkipProtectedWindow(MessageParcel& data, MessageParcel& reply);
    void ProcGetScreenAreaOfDisplayArea(MessageParcel& data, MessageParcel& reply);
    void ProcSetPrimaryDisplaySystemDpi(MessageParcel& data, MessageParcel& reply);
    void ProcSetVirtualScreenAutoRotation(MessageParcel& data, MessageParcel& reply);
    IPCPriority GetIPCPriority(uint32_t code);
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_SCREEN_SESSION_MANAGER_INTERFACE_H