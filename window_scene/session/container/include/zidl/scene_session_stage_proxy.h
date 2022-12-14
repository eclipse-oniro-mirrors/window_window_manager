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

#ifndef OHOS_SCENE_SESSION_STAGE_RPOXY_H
#define OHOS_SCENE_SESSION_STAGE_RPOXY_H

#include <iremote_proxy.h>

#include "scene_session_stage_interface.h"
#include "window_scene_common.h"

namespace OHOS::Rosen {
class SceneSessionStageProxy : public IRemoteProxy<ISceneSessionStage> {
public:
    explicit SceneSessionStageProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<ISceneSessionStage>(impl) {};

    ~SceneSessionStageProxy() {};

    virtual WSError SetActive(bool active) override;
private:
    static inline BrokerDelegator<SceneSessionStageProxy> delegator_;
};
}
#endif // OHOS_SCENE_SESSION_STAGE_RPOXY_H

