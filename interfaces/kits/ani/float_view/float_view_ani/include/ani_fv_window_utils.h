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

#ifndef ANI_FV_WINDOW_UTILS_H
#define ANI_FV_WINDOW_UTILS_H

#include "ani.h"
#include "window.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace Rosen {
class AniFvUtils {
public:
    static ani_object AniThrowError(ani_env* env, WMError wmError, const std::string& message = "");
    static ani_object AniThrowError(ani_env* env, WmErrorCode wmErrorCode, const std::string& message = "");
    static ani_status GetTemplateType(ani_env* env, ani_object floatViewConfigurations, uint32_t& templateType);
    static ani_status GetContextPtr(ani_env* env, ani_object floatViewConfigurations, void*& contextPtr);
    static ani_ref AniGetUndefined(ani_env* env);
    static std::string GetErrorMsg(WMError error);
    static std::string GetErrorMsg(WmErrorCode error);
    static ani_status CreateBusinessError(ani_env* env, int32_t error, std::string message, ani_object* err);
    static ani_status GetStdString(ani_env* env, ani_string ani_str, std::string &result);
    static void* GetAbilityContext(ani_env* env, ani_object aniObj);
    static bool GetNativeAddress(ani_env* env,
        ani_object aniObj, const std::string& className, const std::string& field, ani_long& nativeAddress);
    static ani_status CallAniFunctionVoid(ani_env* env, const char* ns, const char* fn, const char* signature, ...);
    static ani_status GetAniString(ani_env* env, const std::string& str, ani_string* result);
    static bool ParseWindowSize(ani_env* env, ani_object windowSize, Rect& rect);
    static ani_object CreateAniFloatViewStateChangeInfoObject(ani_env* env,
        const FloatViewState state, const std::string& reason);
    static ani_object CreateAniFloatViewRectChangeInfoObject(ani_env* env,
        const Rect& rect, double scale, const std::string& reason);
    static ani_object CreateAniFloatViewPropertiesObject(ani_env* env, uint32_t templateType,
        const sptr<Window>& window, const FloatViewWindowInfo &windowInfo, const FvWindowState &state);
    static ani_object CreateAniFloatViewLimitsObject(ani_env* env, const FloatViewLimits& limits);
    static ani_object CreateAniRatioObject(ani_env* env, const double& min, const double& max);
};
}
}
#endif // ANI_FV_WINDOW_UTILS_H