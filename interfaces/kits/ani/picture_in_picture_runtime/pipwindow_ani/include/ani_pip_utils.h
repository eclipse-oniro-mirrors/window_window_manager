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

namespace OHOS {
namespace Rosen {
ani_ref AniGetUndefined(ani_env* env);
ani_ref AniThrowError(ani_env* env, WMError error, std::string msg = "");
std::string GetErrorMsg(WmErrorCode errorCode);
ani_status GetStdString(ani_env *env, ani_string ani_str, std::string &result);
void* GetAbilityContext(ani_env *env, ani_object aniObj);
ani_status CallAniFunctionVoid(ani_env* env, const char* ns, const char* fn, const char* signature, ...);
ani_status GetAniString(ani_env* env, const std::string& str, ani_string* result);
} // Rosen
} // OHOS
#endif // ANI_PIP_UTILS_H