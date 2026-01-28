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

#ifndef ANI_FB_WINDOW_CONTROLLER_H
#define ANI_FB_WINDOW_CONTROLLER_H

#include <map>
#include <mutex>

#include "ani.h"
#include <refbase.h>
#include "pixel_map.h"
#include "pixel_map_taihe_ani.h"
#include "ani_common_want.h"
#include "window.h"
#include "color_parser.h"
#include "permission.h"

#include "wm_common.h"
#include "window_manager_hilog.h"
#include "floating_ball_controller.h"
#include "floating_ball_manager.h"
#include "ani_fb_window_utils.h"
#include "ani_fb_window_listener.h"

namespace OHOS {
namespace Rosen {
ani_status ANI_Controller_Constructor(ani_vm *vm, uint32_t *result);
ani_ref CreateAniFbControllerObject(ani_env *env, const sptr<FloatingBallController> &fbController);

class AniFbController {
public:
    explicit AniFbController(const sptr<FloatingBallController> &fbController);
    ~AniFbController();

    static constexpr uint32_t TITLE_MIN_LEN = 1;
    static constexpr uint32_t TITLE_MAX_LEN = 64;
    static constexpr uint32_t CONTENT_MAX_LEN = 64;
    static constexpr int32_t PIXEL_MAP_MAX_SIZE = 192 * 1024;

    static void Finalizer(ani_env *env, ani_long nativeObj);
    ani_ref GetAniRef() {return aniRef_;}
    void SetAniRef(const ani_ref &aniRef) {aniRef_ = aniRef;}
    static void StartFloatingBallAni(ani_env *env, ani_object obj, ani_long nativeObj, ani_object paramsInterface);
    static void UpdateFloatingBallAni(ani_env *env, ani_object obj, ani_long nativeObj, ani_object paramsInterface);
    static void StopFloatingBallAni(ani_env *env, ani_object obj, ani_long nativeObj);
    static ani_object GetFloatingBallWindowInfoAni(ani_env *env, ani_object obj, ani_long nativeObj);
    static void RestoreMainWindowAni(ani_env *env, ani_object, ani_long nativeObj, ani_object want);
    static void RegisterFbOnClickCallback(ani_env *env, ani_object, ani_long nativeObj, ani_ref callback);
    static void RegisterFbOnStateChangeCallback(ani_env *env, ani_object, ani_long nativeObj, ani_ref callback);
    static void UnRegisterFbOnClickCallback(ani_env *env, ani_object obj, ani_long nativeObj, ani_ref callback);
    static void UnRegisterFbOnStateChangeCallback(ani_env *env, ani_object obj, ani_long nativeObj, ani_ref callback);

private:
    sptr<FloatingBallController> fbController_ = nullptr;
    ani_ref aniRef_ = nullptr;
    enum class FbListenerType : uint32_t {
        CLICK_CB = 0,
        STATE_CHANGE_CB = 1,
    };
    static inline std::unordered_map<FbListenerType, std::unordered_map<ani_ref, sptr<AniFbWindowListener>>>
        typeCallbackListenerMap_;
    static inline std::mutex mtxListener_;

    bool CheckParams(ani_env* env, const FbOption& option);
    void OnstartFloatingBallAni(ani_env *env, ani_object paramsInterface);
    void OnupdateFloatingBallAni(ani_env *env, ani_object paramsInterface);
    bool OnstopFloatingBallAni(ani_env* env);
    ani_object OnGetFloatingBallWindowInfoAni(ani_env *env);
    ani_object CreateFloatingBallWindowInfoObject(ani_env *env, uint32_t &windowId);
    void OnrestoreMainWindowAni(ani_env *env, ani_object want);
    bool IsCallbackRegistered(ani_env *env, FbListenerType fbListenerType, ani_ref callback);
    void OnRegisterFbOnClickCallback(ani_env *env, ani_ref callback, AniFbController *&aniFbController);
    WmErrorCode RegisterListenerOnClick(ani_env *env, ani_ref callback);
    WmErrorCode ProcessOnClickListener(sptr<AniFbWindowListener> &listener);
    void OnRegisterFbOnStateChangeCallback(ani_env *env, ani_ref callback, AniFbController *&aniFbController);
    WmErrorCode RegisterListenerOnStateChange(ani_env *env, ani_ref callback);
    WmErrorCode ProcessOnStateChangeListener(sptr<AniFbWindowListener> &listener);
    void OnUnRegisterCallback(ani_env *env, FbListenerType fbListenerType, ani_ref callback,
                              AniFbController *&aniFbController);
    WMError UnRegisterListener(FbListenerType fbListenerType, sptr<AniFbWindowListener> &listener);
    void LogTypeCallbackListenerMapSize(FbListenerType fbListenerType);
};  // class AniFbController
}  // namespace Rosen
}  // namespace OHOS
#endif  // ANI_FB_WINDOW_CONTROLLER_H