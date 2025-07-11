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

#ifndef OHOS_ANI_WINDOW_STAGE_H
#define OHOS_ANI_WINDOW_STAGE_H

#include "ani.h"
#include "window_scene.h"

namespace OHOS {
namespace Rosen {
#ifdef _WIN32
#define WINDOW_EXPORT __attribute__((dllexport))
#else
#define WINDOW_EXPORT __attribute__((visibility("default")))
#endif

class AniWindowStage {
    public:
    explicit AniWindowStage(const std::shared_ptr<Rosen::WindowScene>& windowScene);
    ~AniWindowStage();
    static void DisableWindowDecor(ani_env* env, ani_object obj, ani_long nativeObj);
    static void SetShowOnLockScreen(ani_env* env, ani_class cls, ani_long nativeObj, ani_boolean showOnLockScreen);

    void LoadContent(ani_env* env, const std::string& content);
    std::weak_ptr<WindowScene> GetWindowScene() { return windowScene_; }
    ani_object GetMainWindow(ani_env* env);
    ani_boolean WindowIsWindowSupportWideGamut(ani_env* env, ani_class cls, ani_object obj);
private:
    void OnDisableWindowDecor(ani_env* env);
    void OnSetShowOnLockScreen(ani_env* env, ani_boolean showOnLockScreen);
    std::weak_ptr<WindowScene> windowScene_;
};

ani_object CreateAniWindowStage(ani_env* env, std::shared_ptr<Rosen::WindowScene>& windowScene);
AniWindowStage* GetWindowStageFromAni(void* aniObj);
void DropWindowStageByAni(ani_object obj);
}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_ANI_WINDOW_STAGE_H
