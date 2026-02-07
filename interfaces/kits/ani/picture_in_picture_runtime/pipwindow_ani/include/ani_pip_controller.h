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

#ifndef ANI_PIP_CONTROLLER_H
#define ANI_PIP_CONTROLLER_H

#include <map>
#include <unordered_map>

#include <refbase.h>
#include "picture_in_picture_option_ani.h"
#include "picture_in_picture_manager.h"
#include "wm_common.h"
#include "window_manager_hilog.h"
#include "picture_in_picture_controller_ani.h"
#include "ani_pip_listener.h"

namespace OHOS {
namespace Rosen {
ani_ref CreateAniPiPControllerObject(ani_env* env, sptr<PictureInPictureControllerAni>& pipController);
class AniPipController {
public:
    explicit AniPipController(const sptr<PictureInPictureControllerAni>& pipController);
    ~AniPipController();

    static void Finalizer(ani_env* env, ani_long nativeObj);
    void SetAniRef(const ani_ref& aniRef) { aniRef_ = aniRef; }
    ani_ref GetAniRef() { return aniRef_; }
    static void DelListener(ani_env* env);

    static void StartPiPAni(ani_env* env, ani_object obj, ani_long nativeObj);
    static void StopPiPAni(ani_env* env, ani_object obj, ani_long nativeObj);
    static ani_boolean IsPiPSupportedAni(ani_env* env, ani_object obj, ani_long nativeObj);
    static void SetAutoStartEnabledAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable);
    static void UpdateContentSizeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_int w, ani_int h);
    static void UpdatePiPControlStatusAni(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_int controlType, ani_int status);
    static void UpdateContentNodeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_object typeNode);
    static void SetPiPControlEnabledAni(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_int controlType, ani_boolean enabled);
    static ani_ref GetPiPWindowInfoAni(ani_env* env, ani_object obj, ani_long nativeObj);
    static ani_ref OnGetPiPWindowInfoAni(ani_env* env, const sptr<Window>& pipWindow, Rect r, float maxScale);
    static bool GetPiPSettingSwitchAni(ani_env* env, ani_object obj, ani_long nativeObj);
    static bool IsPiPActiveAni(ani_env* env, ani_object obj, ani_long nativeObj);
    static void OnStateChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback);
    static void OnControlPanelActionEventAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback);
    static void OnControlEventAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback);
    static void OnPipWindowSizeChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback);
    static void OnActiveStatusChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback);
    static void OffStateChangeAni(ani_env* env, ani_object obj, ani_long nativeObj);
    static void OffControlPanelActionEventAni(ani_env* env, ani_object obj, ani_long nativeObj);
    static void OffControlEventAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback);
    static void OffPipWindowSizeChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback);
    static void OffActiveStatusChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback);

private:
    enum class AniListenerType : uint32_t {
        STATE_CHANGE_CB,
        CONTROL_PANEL_ACTION_EVENT_CB,
        CONTROL_EVENT_CB,
        SIZE_CHANGE_CB,
        ACTIVE_STATUS_CHANGE_CB
    };

    void OnstartPiPAni(ani_env* env);
    void OnstopPiPAni(ani_env* env);
    bool IsCallbackRegistered(ani_env* env, AniListenerType type, ani_ref cb);
    WMError RegisterListener(ani_env* env, AniListenerType type, ani_ref cb);
    WMError UnregisterListener(ani_env* env, AniListenerType type, ani_ref cb);
    WMError ClearListener(const AniListenerType& type, sptr<AniPiPListener> listener);
    void ProcessRegisterListener(AniListenerType type, sptr<AniPiPListener> listener, WMError& ret);

    static inline std::mutex mtxListener_;
    static inline std::unordered_map<AniListenerType, std::unordered_map<ani_ref, sptr<AniPiPListener>>>
        typeCallbackListenerMap_;
    sptr<PictureInPictureControllerAni> pipController_ = nullptr;
    ani_ref aniRef_ = nullptr;
};

ani_status ANI_Controller_Constructor(ani_vm *vm, uint32_t *result);
} // namespace Rosen
} // namespace OHOS

#endif // ANI_PIP_CONTROLLER_H