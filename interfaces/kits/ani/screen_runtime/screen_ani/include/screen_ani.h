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
#ifndef OHOS_ANI_SCREEN_H
#define OHOS_ANI_SCREEN_H
 
#include "ani.h"
#include "screen.h"
 
namespace OHOS {
namespace Rosen {
 
class ScreenAni {
public:
    explicit ScreenAni(const sptr<Screen>& screen);
    static void SetDensityDpi(ani_env* env, ani_object obj, ani_double densityDpi);
    void OnSetDensityDpi(ani_env* env, ani_object obj, ani_double densityDpi);
    static ani_boolean TransferStatic(ani_env* env, ani_object obj, ani_object input, ani_object screenAniObj);
    static ani_object TransferDynamic(ani_env* env, ani_object obj, ani_long nativeObj);
    sptr<Screen> GetScreen() const { return screen_; }
    static void SetScreenActiveMode(ani_env* env, ani_object obj, ani_long modeIndex);
    void OnSetScreenActiveMode(ani_env* env, ani_object obj, ani_long modeIndex);
    static void SetOrientation(ani_env* env, ani_object obj, ani_enum_item orientationAni);
    void OnSetOrientation(ani_env* env, ani_object obj, ani_enum_item orientationAni);
 
private:
    sptr<Screen> screen_ = nullptr;
};
}
}
#endif
