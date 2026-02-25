/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#ifndef ANI_FBWINDOW_H
#define ANI_FBWINDOW_H

#include "ani.h"

#include "floating_ball_option.h"
#include "ani_fb_window_controller.h"

namespace OHOS {
namespace Rosen {
class AniFbWindow {
public:
    AniFbWindow();
    ~AniFbWindow();

    static void AddAniFbControllerObj(AniFbController* aniFbController, ani_object obj);
    static ani_status DelAniFbControllerObj(AniFbController* aniFbController);

    ani_ref GetAniRef() { return aniRef_; }
    void SetAniRef(const ani_ref& aniRef) { aniRef_ = aniRef; }
    static ani_status Init(ani_env* env, ani_namespace nsp);
    static ani_ref Create(ani_env* env, ani_long nativeObj, ani_object floatingBallConfigurations);
    static ani_boolean IsFloatingBallEnabledAni(ani_env* env);
private:
    static inline std::unordered_map<AniFbController*, ani_object> localObjs;
    static inline std::mutex mtxLocalObjs_;

    ani_ref OnCreate(ani_env* env, ani_object floatingBallConfigurations);
    ani_ref CreateFbController(ani_env* env, void* contextPtr);
    ani_ref aniRef_ = nullptr;
};  // class AniFbWindow
}  // namespace Rosen
}  // namespace OHOS
#endif