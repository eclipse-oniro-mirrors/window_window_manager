/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ANI_SCENE_SESSION_UTILS_H
#define OHOS_ANI_SCENE_SESSION_UTILS_H
#include "ani.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {

class AniSceneSessionUtils {
public:
    static ani_object CreateAniUndefined(ani_env* env);
    static ani_object AniThrowError(ani_env* env, WmErrorCode errorCode, std::string msg = "");
    static ani_status NewAniObject(ani_env* env, ani_class cls, const char *signature, ani_object* result, ...);
};
}
}
#endif