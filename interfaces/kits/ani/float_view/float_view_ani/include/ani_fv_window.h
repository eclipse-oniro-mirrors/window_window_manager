/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#ifndef ANI_FV_WINDOW_H
#define ANI_FV_WINDOW_H

#include "ani.h"

#include "float_view_option.h"
#include "ani_fv_window_controller.h"

namespace OHOS {
namespace Rosen {
class AniFvWindow {
public:
    AniFvWindow();
    ~AniFvWindow();

    static void AddAniFvControllerObj(AniFvController* aniFvController, ani_object obj);
    static ani_status DelAniFvControllerObj(AniFvController* aniFvController);
    ani_ref GetAniRef() { return aniRef_; }
    void SetAniRef(const ani_ref& aniRef) { aniRef_ = aniRef; }
    static ani_status Init(ani_env* env, ani_namespace nsp);

    static ani_ref Create(ani_env* env, ani_long nativeObj, ani_object floatViewConfigurations);
    static ani_boolean IsFloatViewEnabledAni(ani_env* env);
    static ani_object GetFloatViewLimitsAni(ani_env* env, ani_int templateType);
    static ani_object BindAni(ani_env* env, ani_object fvController, ani_object fbController, ani_object aniFbOption);
    static ani_object UnBindAni(ani_env* env, ani_object fvController, ani_object fbController);
private:
    static inline std::unordered_map<AniFvController*, ani_object> localObjs;
    static inline std::mutex mtxLocalObjs_;

    ani_ref OnCreate(ani_env* env, ani_object floatViewConfigurations);
    ani_ref CreateFvController(ani_env* env, void* contextPtr, uint32_t templateType);
    ani_ref aniRef_ = nullptr;
};  // class AniFvWindow
}  // namespace Rosen
}  // namespace OHOS
#endif