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
 * WITHOUT WARRANTIES, CONDITIONS OF ANY KIND, either express or implied.

 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANI_FV_WINDOW_CONTROLLER_H
#define ANI_FV_WINDOW_CONTROLLER_H

#include "float_view_controller.h"

#include "ani.h"

#include "ani_fv_window_listener.h"

namespace OHOS {
namespace Rosen {
auto GetNativeMethod();
ani_status ANI_Controller_Constructor(ani_vm *vm, uint32_t *result);
ani_ref CreateAniFvControllerObject(ani_env *env, const sptr<FloatViewController> &fvController);

class AniFvController {
public:
    explicit AniFvController(const sptr<FloatViewController> &fvController);
    ~AniFvController();

    static void Finalizer(ani_env *env, ani_long nativeObj);
    ani_ref GetAniRef() {return aniRef_;}
    void SetAniRef(const ani_ref &aniRef) {aniRef_ = aniRef;}
    sptr<FloatViewController> GetController() const;
    static void StartFloatViewAni(ani_env *env, ani_object obj, ani_long nativeObj);
    static void StopFloatViewAni(ani_env *env, ani_object obj, ani_long nativeObj);
    static void SetUIContextAni(ani_env *env, ani_object obj, ani_long nativeObj,
        ani_string contextUrl, ani_object contentStorage);
    static void SetFloatViewVisibilityInAppAni(ani_env *env, ani_object obj, ani_long nativeObj,
        ani_boolean visibleInApp);
    static void SetWindowSizeAni(ani_env *env, ani_object obj, ani_long nativeObj, ani_object rectObj);
    static ani_object GetWindowPropertiesAni(ani_env *env, ani_object obj, ani_long nativeObj);
    static void RestoreMainWindowAni(ani_env *env, ani_object obj, ani_long nativeObj, ani_object wantParameters);
    static void OnStateChangeAni(ani_env *env, ani_object, ani_long nativeObj, ani_ref callback);
    static void OnRectChangeAni(ani_env *env, ani_object, ani_long nativeObj, ani_ref callback);
    static void OnLimitsChangeAni(ani_env *env, ani_object, ani_long nativeObj, ani_ref callback);
    static void OffFvOnStateChangeAni(ani_env *env, ani_object obj, ani_long nativeObj, ani_ref callback);
    static void OffRectChangeAni(ani_env *env, ani_object obj, ani_long nativeObj, ani_ref callback);
    static void OffOnLimitsChangeAni(ani_env *env, ani_object obj, ani_long nativeObj, ani_ref callback);

private:
    sptr<FloatViewController> fvController_ = nullptr;
    ani_ref aniRef_ = nullptr;
    enum class CallbackType : uint32_t {
        STATE_CHANGE_CB = 0,
        RECT_CHANGE_CB = 1,
        LIMITS_CHANGE_CB = 2,
    };
    static inline std::unordered_map<CallbackType, std::unordered_map<ani_ref, sptr<AniFvWindowListener>>>
        typeCallbackListenerMap_;
    static inline std::mutex mtxListener_;

    void OnStartFloatViewAni(ani_env *env);
    void OnStopFloatViewAni(ani_env* env);
    void OnSetUIContextAni(ani_env *env, ani_string contextUrl, ani_object contentStorage);
    void OnSetFloatViewVisibilityInAppAni(ani_env *env, ani_boolean visibleInApp);
    void OnSetWindowSizeAni(ani_env *env, ani_object rectObj);
    ani_object OnGetWindowPropertiesAni(ani_env *env);
    void OnRestoreMainWindowAni(ani_env *env, ani_object wantParameters);
    bool IsCallbackRegistered(ani_env *env, CallbackType callbackType, ani_ref callback);
    void RegisterCallbackWithType(ani_env* env, CallbackType callbackType, ani_ref callback);
    void UnregisterCallbackWithType(ani_env* env, CallbackType callbackType, ani_ref callback);
    WMError DoRegisterListenerWithType(CallbackType callbackType, sptr<AniFvWindowListener>& listener);
    WMError DoUnRegisterListenerWithType(CallbackType callbackType, sptr<AniFvWindowListener>& listener);
};  // class AniFvController
}  // namespace Rosen
}  // namespace OHOS
#endif  // ANI_FV_WINDOW_CONTROLLER_H