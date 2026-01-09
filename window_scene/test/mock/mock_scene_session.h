/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_ROSEN_WINDOW_SCENE_MOCK_SCENE_SESSION_H
#define OHOS_ROSEN_WINDOW_SCENE_MOCK_SCENE_SESSION_H
#include "interfaces/include/ws_common.h"
#include "session/host/include/scene_session.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace Rosen {
class SceneSessionMocker : public SceneSession {
public:
    SceneSessionMocker(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback)
        : SceneSession(info, specificCallback) {}
    ~SceneSessionMocker() {}
    MOCK_METHOD(void, CheckAndMoveDisplayIdRecursively, (uint64_t displayId), (override));
    MOCK_METHOD(void, SetScreenId, (uint64_t screenId), (override));
    MOCK_METHOD(void, UpdateCrossAxisOfLayout, (const WSRect& rect), (override));
    MOCK_METHOD(void, UpdateCrossAxis, (), (override));
    MOCK_METHOD(WSError, UpdateGlobalDisplayRect, (const WSRect& rect, SizeChangeReason reason), (override));
    MOCK_METHOD(void, SetSurfaceBounds, (const WSRect& rect, bool isGlobal, bool needFlush), (override));
};
}
}
#endif // OHOS_ROSEN_WINDOW_SCENE_MOCK_SCENE_SESSION_H