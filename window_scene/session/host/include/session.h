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

#ifndef OHOS_SESSION_H
#define OHOS_SESSION_H

#include <string>
#include <refbase.h>
#include <ui/rs_surface_node.h>
#include "window_scene_common.h"

namespace OHOS::Rosen {
class Session {
public:
    Session(const std::string& name);
    ~Session() = default;

    void SetPersistentId(uint32_t persistentId);
    uint32_t GetPersistentId() const;
    std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const;
    SessionState GetSessionState() const;
    virtual WSError SetActive(bool active) = 0;

protected:
    void UpdateSessionState(SessionState state);

private:
    RSSurfaceNode::SharedPtr CreateSurfaceNode(std::string name);
    uint32_t persistentId_ = INVALID_SESSION_ID;
    std::shared_ptr<RSSurfaceNode> surfaceNode_ = nullptr;
    SessionState state_ = SessionState::STATE_INITIAL;
};
}
#endif // OHOS_SESSION_H
