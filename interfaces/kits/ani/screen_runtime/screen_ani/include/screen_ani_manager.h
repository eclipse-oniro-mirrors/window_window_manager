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
#ifndef SCREEN_MANAGER_ANI_H
#define SCREEN_MANAGER_ANI_H

#include <mutex>

#include "ani.h"
#include "dm_common.h"
#include "refbase.h"
#include "screen_ani_listener.h"
#include "screen_manager.h"

namespace OHOS {
namespace Rosen {

class ScreenManagerAni {
public:
    explicit ScreenManagerAni();
    static void RegisterCallback(ani_env* env, ani_string type,
        ani_ref callback, ani_long nativeObj);
    static void UnRegisterCallback(ani_env* env, ani_string type,
        ani_long nativeObj, ani_ref callback);
    static ani_status InitScreenManagerAni(ani_namespace screenNameSpace, ani_env* env);
    DmErrorCode ProcessRegisterCallback(ani_env* env, std::string& typeStr,
        sptr<ScreenAniListener> screenAniListener);
    static ani_long MakeMirror(ani_env* env, ani_long mainScreen, ani_object mirrorScreen);
    static void GetAllScreens(ani_env* env, ani_object screensAni);
    static void CreateVirtualScreen(ani_env* env, ani_object options, ani_object virtualScreen);
    static void SetVirtualScreenSurface(ani_env* env, ani_long screenId, ani_string surfaceIdAni);
    static void DestroyVirtualScreen(ani_env* env, ani_long screenId);
    static ani_boolean IsScreenRotationLocked(ani_env* env);
    static void SetScreenRotationLocked(ani_env* env, ani_boolean isLocked);
    static void SetMultiScreenRelativePosition(ani_env* env, ani_object mainScreenOptionsAni,
        ani_object secondaryScreenOptionsAni);
    static void SetMultiScreenMode(ani_env* env, ani_long primaryScreenId, ani_long secondaryScreenId,
        ani_enum_item secondaryScreenMode);
    static void SetScreenPrivacyMaskImage(ani_env* env, ani_long screenId, ani_object imageAni);
    static ani_object MakeUnique(ani_env* env, ani_object uniqueScreenIds);
    static ani_long MakeMirrorWithRegion(ani_env* env, ani_long mainScreen, ani_object mirrorScreen,
        ani_object mainScreenRegionAni);
    static void StopMirror(ani_env* env, ani_object mirrorScreen);
    static ani_status NspBindNativeFunctions(ani_env* env, ani_namespace nsp);
    static ani_status ClassBindNativeFunctions(ani_env* env, ani_class screenCls);
    static ani_long MakeExpand(ani_env* env, ani_object expandOptionsAni);
    static void StopExpand(ani_env* env, ani_object expandScreensAni);
private:
    void OnRegisterCallback(ani_env* env, ani_string type, ani_ref callback);
    void OnUnRegisterCallback(ani_env* env, ani_string type, ani_ref callback);
    DMError UnRegisterScreenListenerWithType(std::string type, ani_env* env, ani_ref callback);
    DMError UnRegisterAllScreenListenerWithType(std::string type);
    bool IsCallbackRegistered(ani_env* env, const std::string& type, ani_ref callback);
    std::mutex mtx_;
    std::map<std::string, std::map<ani_ref, sptr<ScreenAniListener>>> jsCbMap_;
};
}
}
#endif