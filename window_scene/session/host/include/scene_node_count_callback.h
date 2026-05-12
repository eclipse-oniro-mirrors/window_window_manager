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

#ifndef OHOS_WINDOW_SCENE_SCENE_NODE_COUNT_CALLBACK_H
#define OHOS_WINDOW_SCENE_SCENE_NODE_COUNT_CALLBACK_H

#include "future.h"
#include "zidl/scene_node_count_callback_stub.h"

namespace OHOS {
namespace Rosen {
class SceneNodeCountCallback : public SceneNodeCountCallbackStub {
public:
    SceneNodeCountCallback() = default;
    ~SceneNodeCountCallback() = default;
    
    void OnSceneNodeCount(uint32_t nodeCount) override;
    uint32_t GetResult(long timeoutMs);
    void ResetResult();
    
private:
    RunnableFuture<uint32_t> future_{};
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_WINDOW_SCENE_SCENE_NODE_COUNT_CALLBACK_H
