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

#ifndef MOCK_WINDOW_SCENE_SESSION_IMPL_H
#define MOCK_WINDOW_SCENE_SESSION_IMPL_H

#include <gmock/gmock.h>

#include "window_scene_session_impl.h"

namespace OHOS {
namespace Rosen {
class MockWindowSceneSessionImpl : public WindowSceneSessionImpl {
public:
    MockWindowSceneSessionImpl(const sptr<WindowOption>& option,
        const std::shared_ptr<RSUIContext>& rsUIContext = nullptr) : WindowSceneSessionImpl(option, rsUIContext) {}
    ~MockWindowSceneSessionImpl() {}

    MOCK_METHOD(float, GetVirtualPixelRatio, (const sptr<DisplayInfo>& displayInfo), (override));
};
} // Rosen
} // OHOS
#endif // MOCK_WINDOW_SCENE_SESSION_IMPL_H