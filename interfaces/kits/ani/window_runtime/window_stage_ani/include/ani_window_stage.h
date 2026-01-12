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
#include "ani_window_register_manager.h"
#include "window_scene.h"

namespace OHOS {
namespace Rosen {
#ifdef _WIN32
#define WINDOW_EXPORT __attribute__((dllexport))
#else
#define WINDOW_EXPORT __attribute__((visibility("default")))
#endif

enum class ImageFit {
    FILL = 0,
    CONTAIN,
    COVER,
    FITWIDTH,
    FITHEIGHT,
    NONE,
    SCALE_DOWN,
    TOP_LEFT,
    TOP,
    TOP_END,
    START,
    CENTER,
    END,
    BOTTOM_START,
    BOTTOM,
    BOTTOM_END,
    MATRIX,
};

enum class Ark_ImageFit {
    ARK_IMAGE_FIT_CONTAIN = 0,
    ARK_IMAGE_FIT_COVER = 1,
    ARK_IMAGE_FIT_AUTO = 2,
    ARK_IMAGE_FIT_FILL = 3,
    ARK_IMAGE_FIT_SCALE_DOWN = 4,
    ARK_IMAGE_FIT_NONE = 5,
    ARK_IMAGE_FIT_TOP_START = 7,
    ARK_IMAGE_FIT_TOP = 8,
    ARK_IMAGE_FIT_TOP_END = 9,
    ARK_IMAGE_FIT_START = 10,
    ARK_IMAGE_FIT_CENTER = 11,
    ARK_IMAGE_FIT_END = 12,
    ARK_IMAGE_FIT_BOTTOM_START = 13,
    ARK_IMAGE_FIT_BOTTOM = 14,
    ARK_IMAGE_FIT_BOTTOM_END = 15,
    ARK_IMAGE_FIT_MATRIX = 16,
};
class AniWindowStage {
public:
    explicit AniWindowStage(const std::shared_ptr<Rosen::WindowScene>& windowScene);
    ~AniWindowStage();
    static ani_object NativeTransferStatic(ani_env* aniEnv, ani_class cls, ani_object input);
    static ani_object NativeTransferDynamic(ani_env* aniEnv, ani_class cls, ani_long nativeObj);
    static void LoadContent(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_string path, ani_object storage);
    static void LoadContentByName(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_string path, ani_object storage);
    static void DisableWindowDecor(ani_env* env, ani_object obj, ani_long nativeObj);
    static void SetShowOnLockScreen(ani_env* env, ani_class cls, ani_long nativeObj, ani_boolean showOnLockScreen);
    static void SetWindowModal(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isModal);
    static void SetWindowRectAutoSave(ani_env* env, ani_class cls, ani_long nativeObj,
        ani_boolean enable, bool isSaveBySpecifiedFlag);
    static ani_boolean IsWindowRectAutoSave(ani_env* env, ani_class cls, ani_long nativeObj);
    static void RegisterWindowCallback(ani_env* env, ani_object obj, ani_long nativeObj, ani_string type,
        ani_ref callback);
    static void UnregisterWindowCallback(ani_env* env, ani_object obj, ani_long nativeObj, ani_string type,
        ani_ref callback);

    static void SetImageForRecent(ani_env* env, ani_class cls, ani_long nativeObj, ani_object imageResource,
        ani_int value);
    static void RemoveImageForRecent(ani_env* env, ani_class cls, ani_long nativeObj);
    static void SetCustomDensity(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_double density, ani_boolean applyToSubWindow);
    static void SetDefaultDensityEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled);
    static ani_object GetSubWindow(ani_env* env, ani_object obj, ani_long nativeObj);
    static ani_object CreateSubWindowWithOptions(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_string name, ani_object options);
    static void RemoveStartingWindow(ani_env* env, ani_object obj, ani_long nativeObj);
    static void SetSupportedWindowModes(
        ani_env* env, ani_object obj, ani_long nativeObj, ani_object aniSupportedWindowModes);
    static void SetSupportedWindowModesWithGrayOutMaximizeButton(
        ani_env* env, ani_object obj, ani_long nativeObj,
        ani_object aniSupportedWindowModes, ani_boolean grayOutMaximizeButton);
    std::weak_ptr<WindowScene> GetWindowScene() { return windowScene_; }
    ani_ref GetMainWindow(ani_env* env);
    ani_boolean WindowIsWindowSupportWideGamut(ani_env* env, ani_class cls, ani_object obj);
    ani_ref OnCreateSubWindow(ani_env *env, ani_string name);
private:
    void OnLoadContent(ani_env* env, ani_string path, ani_object storage, bool isLoadByName);
    void OnDisableWindowDecor(ani_env* env);
    void OnSetShowOnLockScreen(ani_env* env, ani_boolean showOnLockScreen);
    void OnSetWindowModal(ani_env* env, ani_boolean isModal);
    void OnSetWindowRectAutoSave(ani_env* env, ani_boolean enable, bool isSaveBySpecifiedFlag);
    ani_boolean OnIsWindowRectAutoSave(ani_env* env);
    void OnRegisterWindowCallback(ani_env* env, ani_string type, ani_ref callback);
    void OnUnregisterWindowCallback(ani_env* env, ani_string type, ani_ref callback);

    void OnSetImageForRecent(ani_env* env, ani_object imageResource, ani_int value);
    void OnRemoveImageForRecent(ani_env* env);
    void OnSetCustomDensity(ani_env* env, ani_double density, ani_boolean applyToSubWindow);
    void OnSetDefaultDensityEnabled(ani_env* env, ani_boolean enabled);
    void OnSetSupportedWindowModes(ani_env* env, ani_object aniSupportedWindowModes);
    void OnSetSupportedWindowModesWithGrayOutMaximizeButton(
        ani_env* env, ani_object aniSupportedWindowModes, ani_boolean grayOutMaximizeButton);
    ani_object OnGetSubWindow(ani_env* env);
    ani_object OnCreateSubWindowWithOptions(ani_env* env, ani_string name, ani_object options);
    void OnRemoveStartingWindow(ani_env* env);

    static void ConvertImageFit(ImageFit& dst, const Ark_ImageFit& src);
    std::weak_ptr<WindowScene> windowScene_;
    std::unique_ptr<AniWindowRegisterManager> registerManager_ = nullptr;
};

ani_object CreateAniWindowStage(ani_env* env, std::shared_ptr<Rosen::WindowScene>& windowScene);
AniWindowStage* GetWindowStageFromAni(void* aniObj);
void DropWindowStageByAni(ani_object obj);
}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_ANI_WINDOW_STAGE_H
