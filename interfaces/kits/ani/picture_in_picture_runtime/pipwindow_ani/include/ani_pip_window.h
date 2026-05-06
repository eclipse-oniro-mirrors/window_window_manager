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
#ifndef ANI_PIPWINDOW_H
#define ANI_PIPWINDOW_H

#include "ani.h"
#include "picture_in_picture_option_ani.h"
#include "ani_pip_controller.h"
#include "picture_in_picture_manager.h"
#include "picture_in_picture_controller_ani.h"

namespace OHOS {
namespace Rosen {

class AniPiPWindow {
public:
    AniPiPWindow();
    ~AniPiPWindow();

    static void Finalizer(ani_env* env, ani_long nativeObj);
    static ani_status Init(ani_env* env, ani_namespace nsp);
    void SetPiPController(sptr<PictureInPictureControllerAni> pipControllerToken);
    static ani_ref Create(ani_env* env, ani_long nativeObj, ani_object pipconfiguration, ani_object nativeXComponent);
    static ani_boolean IsPiPEnabledAni(ani_env* env);
    static ani_object NativeTransferStatic(ani_env* aniEnv, ani_class cls, ani_object input);
    static ani_object NativeTransferDynamic(ani_env* aniEnv, ani_class cls, ani_long nativeObj);
    static ani_status BindNamespaceFunctions(ani_env* env, ani_namespace& nsp);
    static ani_status BindClassMethods(ani_env* env);

private:
    static ani_object CreateAniPiPControllerObject(ani_env* env, sptr<PictureInPictureControllerAni>& pipController);
    static inline std::unordered_map<AniPipController*, ani_object> localObjs;
    static inline std::mutex mtxLocalObjs_;
    static void AddAniPiPControllerObj(AniPipController* pipController, ani_object obj);
    static ani_status DelAniPiPControllerObj(AniPipController* aniPipController);
    static bool checkControlsRules(uint32_t pipTemplateType, std::vector<std::uint32_t> controlGroups);
    static bool checkOptionParams(PipOptionAni& option);
    static bool GetControlGroupFromJs(ani_env* env, ani_ref controlGroup, std::vector<std::uint32_t>& controls);
    ani_ref OnCreate(ani_env* env, ani_object pipconfiguration, ani_object nativeXComponent);
    ani_ref CreatePiPController(ani_env* env, PipOptionAni pipOption);
    static bool OptionSetContext(ani_env* env, ani_object optionObject, PipOptionAni& option);
    static bool OptionSetXComponentController(ani_env* env, ani_object optionObject, PipOptionAni& option);
    static void OptionSetNavigationIdValue(ani_env* env, ani_object optionObject, PipOptionAni& option);
    static void GetWidthFromAniObject(ani_env* env, ani_object optionObject, uint32_t& width);
    static void GetHeightFromAniObject(ani_env* env, ani_object optionObject, uint32_t& height);
    static void OptionSetContentSize(ani_env* env, ani_object optionObject, PipOptionAni& option);
    static void OptionSetPipTemplate(ani_env* env, ani_object optionObject, PipOptionAni& option);
    static void OptionSetControlGroup(ani_env* env, ani_object optionObject, PipOptionAni& option);
    static void OptionSetCustomUIController(ani_env* env, ani_object optionObject, PipOptionAni& option);
    static void OptionSetDefaultWindowSizeType(ani_env* env, ani_object optionObject, PipOptionAni& option);
    static void OptionSetStorage(ani_env* env, ani_object optionObject, PipOptionAni& option);
    static void OptionSetHandleId(ani_env* env, ani_object optionObject, PipOptionAni& option);
    static void OptionSetCornerAdsorptionEnabled(ani_env* env, ani_object optionObject, PipOptionAni& option);
    static bool GetPiPOption(ani_env* env, ani_object optionObject, PipOptionAni& option);
};

enum DefaultWindowSizeType : uint32_t {
    NOT_SET,
    SMALL,
    LARGE,
};
}  // namespace Rosen
}  // namespace OHOS
#endif // ANI_PIPWINDOW_H