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

#ifndef OHOS_ANI_EXTENSION_WINDOW_CONFIG_H
#define OHOS_ANI_EXTENSION_WINDOW_CONFIG_H
#include "ani.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
#ifdef _WIN32
#define WINDOW_EXPORT __attribute__((dllexport))
#else
#define WINDOW_EXPORT __attribute__((visibility("default")))
#endif

WINDOW_EXPORT ani_object CreateAniExtensionWindowConfig(ani_env* env,
    const std::shared_ptr<ExtensionWindowConfig>& extensionWindowConfig);

class AniExtensionWindowConfig {
public:
    explicit AniExtensionWindowConfig(const std::shared_ptr<ExtensionWindowConfig>& extensionWindowConfig);
    ~AniExtensionWindowConfig();
    ani_ref GetAniRef() { return aniRef_; }
    void SetAniRef(const ani_ref& aniRef) { aniRef_ = aniRef; }
    
    static void Finalizer(ani_env* env, ani_long nativeObj);

    ani_string OnGetWindowName(ani_env* env);
    ani_enum_item OnGetWindowAttribute(ani_env* env);
    ani_object OnGetWindowRect(ani_env* env);
    ani_int OnGetWindowRectLeft(ani_env* env);
    ani_int OnGetWindowRectTop(ani_env* env);
    ani_int OnGetWindowRectWidth(ani_env* env);
    ani_int OnGetWindowRectHeight(ani_env* env);
    ani_object OnGetSubWindowOptions(ani_env* env);
    ani_string OnGetSubWindowOptionsTitle(ani_env* env);
    ani_boolean OnGetSubWindowOptionsDecorEnabled(ani_env* env);
    ani_boolean OnGetSubWindowOptionsIsModal(ani_env* env);
    ani_boolean OnGetSubWindowOptionsIsTopmost(ani_env* env);
    ani_object OnGetSystemWindowOptions(ani_env* env);
    ani_enum_item OnGetSystemWindowOptionsWindowType(ani_env* env);

    void OnSetWindowName(ani_env* env, ani_string value);
    void OnSetWindowAttribute(ani_env* env, ani_enum_item value);
    void OnSetWindowRect(ani_env* env, ani_object value);
    void OnSetWindowRectLeft(ani_env* env, ani_int value);
    void OnSetWindowRectTop(ani_env* env, ani_int value);
    void OnSetWindowRectWidth(ani_env* env, ani_int value);
    void OnSetWindowRectHeight(ani_env* env, ani_int value);
    void ParseSubWindowOptionsProperties(ani_env* env, ani_object value, SubWindowOptions& options);
    void OnSetSubWindowOptions(ani_env* env, ani_object value);
    void OnSetSubWindowOptionsTitle(ani_env* env, ani_string value);
    void OnSetSubWindowOptionsDecorEnabled(ani_env* env, ani_boolean value);
    void OnSetSubWindowOptionsIsModal(ani_env* env, ani_boolean value);
    void OnSetSubWindowOptionsIsTopmost(ani_env* env, ani_boolean value);
    void OnSetSystemWindowOptions(ani_env* env, ani_object value);
    void OnSetSystemWindowOptionsWindowType(ani_env* env, ani_enum_item value);

private:
    std::shared_ptr<ExtensionWindowConfig> extensionWindowConfig_;
    ani_ref aniRef_ = nullptr;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ANI_EXTENSION_WINDOW_CONFIG_H