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
#ifndef DISPLAY_ANI_H
#define DISPLAY_ANI_H

#include "ani.h"
#include "display.h"
#include "display_ani_listener.h"

namespace OHOS {
namespace Rosen {

class DisplayAni {
public:
    explicit DisplayAni(const sptr<Display>& display);

    static void GetCutoutInfo(ani_env* env, ani_object obj, ani_object cutoutInfoObj);
    static void GetRoundedCorner(ani_env* env, ani_object obj, ani_object roundedCornerObj);
    static ani_string GetDisplayCapability(ani_env* env);
    static void GetAvailableArea(ani_env* env, ani_object obj, ani_object availableAreaObj);
    static void GetLiveCreaseRegion(ani_env* env, ani_object obj, ani_object foldCreaseRegionObj);
    static ani_boolean HasImmersiveWindow(ani_env* env, ani_object obj);
    static void RegisterCallback(ani_env* env, ani_object obj, ani_string type, ani_ref aniCallback);
    static void UnRegisterCallback(ani_env* env, ani_object obj, ani_string type, ani_ref aniCallback);
    static void CreateDisplayAni(sptr<Display> display, ani_object displayAni, ani_env* env);
    void OnRegisterCallback(ani_env* env, ani_object obj, ani_string type, ani_ref aniCallback);
    void OnUnRegisterCallback(ani_env* env, ani_object obj, ani_string type, ani_ref aniCallback);
    static ani_status NspBindNativeFunctions(ani_env* env, ani_namespace nsp);
    static ani_status ClassBindNativeFunctions(ani_env* env, ani_class displayCls);
    static ani_boolean TransferStatic(ani_env* env, ani_object obj, ani_object input, ani_object displayAniObj);
    static ani_object TransferDynamic(ani_env* env, ani_object obj, ani_long nativeObj);
    sptr<Display> GetDisplay() const { return display_; }
private:
    DMError UnregisterAllDisplayListenerWithType(std::string type);
    DMError UnregisterDisplayListenerWithType(std::string type, ani_env *env, ani_ref aniCallback);
    bool IsCallbackRegistered(ani_env* env, const std::string& type, ani_ref callback);
    sptr<Display> display_ = nullptr;
    std::map<std::string, std::map<ani_ref, sptr<DisplayAniListener>>> jsCbMap_;
    std::mutex mtx_;
};
}  // namespace Rosen
}  // namespace OHOS
#endif