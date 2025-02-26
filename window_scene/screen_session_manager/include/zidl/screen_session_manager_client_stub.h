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

#ifndef OHOS_ROSEN_SCREEN_SESSION_MANAGER_CLIENT_STUB_H
#define OHOS_ROSEN_SCREEN_SESSION_MANAGER_CLIENT_STUB_H

#include <map>

#include <iremote_stub.h>

#include "screen_session_manager_client_interface.h"

namespace OHOS::Rosen {
class ScreenSessionManagerClientStub : public IRemoteStub<IScreenSessionManagerClient> {
public:
    ScreenSessionManagerClientStub();
    virtual ~ScreenSessionManagerClientStub() = default;

    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

private:
    using HandleScreenChange = std::function<int(MessageParcel &, MessageParcel &)>;
    using HandleScreenChangeMap = std::map<ScreenSessionManagerClientMessage, HandleScreenChange>;
    void InitScreenChangeMap();
    int HandleOnScreenConnectionChanged(MessageParcel& data, MessageParcel& reply);
    int HandleOnPropertyChanged(MessageParcel& data, MessageParcel& reply);
    int HandleOnPowerStatusChanged(MessageParcel& data, MessageParcel& reply);
    int HandleOnSensorRotationChanged(MessageParcel& data, MessageParcel& reply);
    int HandleOnScreenOrientationChanged(MessageParcel& data, MessageParcel& reply);
    int HandleOnScreenRotationLockedChanged(MessageParcel& data, MessageParcel& reply);
    int HandleOnDisplayStateChanged(MessageParcel& data, MessageParcel& reply);
    int HandleOnScreenshot(MessageParcel& data, MessageParcel& reply);
    int HandleOnImmersiveStateChanged(MessageParcel& data, MessageParcel& reply);
    int HandleOnSetDisplayNodeScreenId(MessageParcel& data, MessageParcel& reply);
    int HandleOnGetSurfaceNodeIdsFromMissionIdsChanged(MessageParcel& data, MessageParcel& reply);
    int HandleOnUpdateFoldDisplayMode(MessageParcel& data, MessageParcel& reply);
    int HandleSetVirtualPixelRatioSystem(MessageParcel& data, MessageParcel& reply);
    int HandleSwitchUserCallback(MessageParcel& data, MessageParcel& reply);
    int HandleOnFoldStatusChangedReportUE(MessageParcel& data, MessageParcel& reply);
    int HandleOnHoverStatusChanged(MessageParcel& data, MessageParcel& reply);
    int HandleScreenCaptureNotify(MessageParcel& data, MessageParcel& reply);
    int HandleOnCameraBackSelfieChanged(MessageParcel& data, MessageParcel& reply);

    HandleScreenChangeMap HandleScreenChangeMap_ {};
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_SCREEN_SESSION_MANAGER_CLIENT_STUB_H
