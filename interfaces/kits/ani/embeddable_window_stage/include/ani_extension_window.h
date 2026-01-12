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

#ifndef OHOS_ANI_EXTENSION_WINDOW_H
#define OHOS_ANI_EXTENSION_WINDOW_H

#include "ani.h"
#include "ani_extension_window_register_manager.h"
#include "extension_window.h"
#include "extension_window.h"
#include "session_info.h"

namespace OHOS {
namespace Rosen {
#ifdef _WIN32
#define WINDOW_EXPORT __attribute__((dllexport))
#else
#define WINDOW_EXPORT __attribute__((visibility("default")))
#endif

class AniExtensionWindow {
public:
    explicit AniExtensionWindow(const std::shared_ptr<Rosen::ExtensionWindow> extensionWindow, int32_t hostWindowId);
    AniExtensionWindow(const std::shared_ptr<Rosen::ExtensionWindow> extensionWindow,
        sptr<AAFwk::SessionInfo> sessionInfo);
    ani_ref GetAniRef() { return aniRef_; }
    void SetAniRef(const ani_ref& aniRef) { aniRef_ = aniRef; }
    
    static void Finalizer(ani_env* env, ani_long nativeObj);
    static ani_object CreateAniExtensionWindow(ani_env* env, sptr<Rosen::Window> window, int32_t hostWindowId,
        bool isHost = true);
    WMError GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea);
    WmErrorCode OnSetWaterMarkFlag(ani_env* env, ani_boolean enable);
    WmErrorCode OnHidePrivacyContentForHost(ani_env* env, ani_boolean shouldHide);
    WmErrorCode GetExtensionWindowRect(Rect& rect);
    WmErrorCode RegisterListener(ani_env* env, ani_string type,  ani_object fn);
    WmErrorCode UnregisterListener(ani_env* env, ani_string type, ani_object fn);
    WmErrorCode OnHideNonSecureWindows(ani_env* env, ani_boolean shouldHide);
    void OnOccupyEvents(ani_env* env, ani_int eventFlags);
    ani_object OnCreateSubWindowWithOptions(ani_env* env, ani_string name, ani_object subWindowOptions,
        ani_boolean followCreatorLifecycle);
    WmErrorCode OnRegisterRectChangeCallback(ani_env* env, ani_int reason, ani_object fn);
    WmErrorCode OnUnRegisterRectChangeCallback(ani_env* env, ani_object fn);
private:
    bool IsExtensionWindowValid();
    std::shared_ptr<Rosen::ExtensionWindow> extensionWindow_;
    int32_t hostWindowId_ = 0;
    sptr<AAFwk::SessionInfo> sessionInfo_ = nullptr;
    std::unique_ptr<AniExtensionWindowRegisterManager> extensionRegisterManager_ = nullptr;
    ani_ref aniRef_ = nullptr;
};

}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_ANI_EXTENSION_WINDOW_H
