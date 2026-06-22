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

#ifndef ANI_PIP_UTILS_H
#define ANI_PIP_UTILS_H

#include <unordered_map>

#include "js_runtime_utils.h"
#include "window.h"
#include "ani.h"
#include "napi/native_api.h"

#include "picture_in_picture_option.h"
#include "picture_in_picture_option_ani.h"
#include "picture_in_picture_controller.h"
#include "picture_in_picture_controller_ani.h"

namespace OHOS {
namespace Rosen {
class AniPipUtils {
public:
    AniPipUtils() = default;
    ~AniPipUtils() = default;
    static ani_object AniGetUndefined(ani_env* env);
    static ani_ref AniThrowError(ani_env* env, WMError error, std::string msg = "");
    static std::string GetErrorMsg(WmErrorCode errorCode);
    static ani_status GetStdString(ani_env *env, ani_string ani_str, std::string &result);
    static void* GetAbilityContext(ani_env *env, ani_object aniObj);
    static ani_status CallAniFunctionVoid(ani_env* env, const char* ns, const char* fn, const char* signature, ...);
    static ani_status GetAniString(ani_env* env, const std::string& str, ani_string* result);
    static bool TransferToPipOptionAni(ani_env* env, const sptr<PipOption>& pipOption,
        sptr<PipOptionAni>& pipOptionAni);
    static bool TransferToPipOptionNapi(ani_env* env, const sptr<PipOptionAni>& pipOptionAni,
        sptr<PipOption>& pipOption);
    static bool convertNativeRefToAniRef(ani_env* env, const std::shared_ptr<NativeReference>& nativeRef,
        ani_ref& aniRef);
    static void TransferToPipControllerAni(sptr<PictureInPictureController>& pipController,
        sptr<PictureInPictureControllerAni>& pipControllerAni);
    static void TransferToPipControllerNapi(sptr<PictureInPictureControllerAni>& pipControllerAni,
        sptr<PictureInPictureController>& pipController);
    static ani_object ConvertNapiValueToAniObject(ani_env* aniEnv, napi_env napiEnv, napi_value jsValue);
    static void InitVM(ani_env* env);
    static ani_vm* vm_;

private:
    static ani_status CreateBusinessError(ani_env* env, int32_t error, std::string message, ani_object* err);
};

} // Rosen
} // OHOS
#endif // ANI_PIP_UTILS_H