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

#ifndef OHOS_ROSEN_WINDOW_SCENE_MOCK_UI_EFFECT_CONTROLLER_INTERFACE_H
#define OHOS_ROSEN_WINDOW_SCENE_MOCK_UI_EFFECT_CONTROLLER_INTERFACE_H
#include <gmock/gmock.h>

#include "../../session/host/include/ui_effect_controller_stub.h"

namespace OHOS {
namespace Rosen {
class UIEffectControllerStubMocker : public UIEffectControllerStub {
public:
    MOCK_METHOD(WMError, SetParams, (const sptr<UIEffectParams>& params), (override));
    MOCK_METHOD(WMError, AnimateTo, ((const sptr<UIEffectParams>& params), (const sptr<WindowAnimationOption>& config),
        (const sptr<WindowAnimationOption>& interruptedOption)), (override));
};
}
}
#endif // OHOS_ROSEN_WINDOW_SCENE_MOCK_SUB_SESSION_H
