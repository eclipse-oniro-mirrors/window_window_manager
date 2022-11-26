/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_SCENE_SESSION_STAGE_STUB_H
#define OHOS_SCENE_SESSION_STAGE_STUB_H

#include <map>
#include <iremote_stub.h>
#include "zidl/scene_session_stage_interface.h"

namespace OHOS::Rosen {
class SceneSessionStageStub;
using SceneSessionStageStubFunc = int (SceneSessionStageStub::*)(MessageParcel& data, MessageParcel& reply);

class SceneSessionStageStub : public IRemoteStub<ISceneSessionStage> {
public:
    SceneSessionStageStub() = default;
    ~SceneSessionStageStub() = default;

    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;

private:
    static const std::map<uint32_t, SceneSessionStageStubFunc> stubFuncMap_;

    int HandleSetActive(MessageParcel& data, MessageParcel& reply);
};
}
#endif // OHOS_SCENE_SESSION_STAGE_STUB_H
