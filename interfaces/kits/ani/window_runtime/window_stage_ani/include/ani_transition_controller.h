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

#ifndef OHOS_ANI_TRANSITION_CONTROLLER_H
#define OHOS_ANI_TRANSITION_CONTROLLER_H

#include "ani.h"
#include "ani_window.h"
#include "event_handler.h"
#include "window.h"

namespace OHOS {
namespace Rosen {
class AniTransitionContext {
public:
    AniTransitionContext(sptr<Window> window, bool isShownTransContext);
    ~AniTransitionContext();

    static ani_status AniTransitionContextInit(ani_env* env);
    static ani_object CreateAniObj(ani_env* env, ani_ref aniWindowObj,
        sptr<Window> window, ani_boolean isShownTransitionContext);
    static void Finalizer(ani_env* env, ani_long nativeObj);
    static void CompleteTransition(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isCompleted);

private:
    void OnCompleteTransition(ani_env* env, ani_boolean isCompleted);
    wptr<Window> windowToken_ = nullptr;
    bool isShownTransContext_ = false;
}; // AniTransitionContext

class AniTransitionController : public IAnimationTransitionController {
public:
    AniTransitionController(ani_env* env, ani_ref aniWindowObj, sptr<Window> window);
    ~AniTransitionController();
    ani_object GetAniTransControllerObj() const;
    void SetAniTransControllerObj(const ani_object& aniObj);
    ani_env* GetEnv() const { return env_; };
    ani_ref GetAniWindowObj() const { return aniWindowObj_; };
    wptr<Window> GetWindowToken() const { return windowToken_; };

    static ani_status AniTransitionControllerInit(ani_env* env);
    static sptr<AniTransitionController> CreateAniTransitionController(ani_env* env,
        ani_ref aniWindowObj, sptr<Window> window);
    static void Finalizer(ani_env* env, ani_long nativeObj);

    void AnimationForShown() override;
    void AnimationForHidden() override;

private:
    ani_object aniTransControllerObj_ = nullptr;
    ani_env* env_ = nullptr;
    ani_ref aniWindowObj_ = nullptr;
    wptr<Window> windowToken_ = nullptr;
    wptr<AniTransitionController> weakRef_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
}; // AniTransitionController

ani_status ANI_Transition_Controller_Constructor(ani_vm* vm, uint32_t *result);
} // Rosen
} // OHOS
#endif // OHOS_ANI_TRANSITION_CONTROLLER_H