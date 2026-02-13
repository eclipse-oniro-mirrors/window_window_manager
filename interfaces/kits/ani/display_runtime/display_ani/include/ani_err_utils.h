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

#ifndef DISPLAY_MANAGER_ANI_ERROR_UTILS_H
#define DISPLAY_MANAGER_ANI_ERROR_UTILS_H

#include "ani.h"
#include "dm_common.h"

namespace OHOS::Rosen {
class AniErrUtils {
public:
    static ani_object CreateAniError(ani_env* env, const DMError& errorCode, const std::string& msg = "");
    static ani_object CreateAniError(ani_env* env, const DmErrorCode& errorCode, const std::string& msg = "");
    static ani_status ThrowBusinessError(ani_env* env, DMError error, std::string message);
    static ani_status ThrowBusinessError(ani_env* env, DmErrorCode error, std::string message);
    static ani_status CreateBusinessError(ani_env* env, int32_t error, std::string message, ani_object* err);

private:
    static std::string GetErrorMsg(const DMError& errorCode);
    static std::string GetErrorMsg(const DmErrorCode& errorCode);
};
} // namespace OHOS::Rosen

#endif //DISPLAY_MANAGER_ANI_ERROR_UTILS_H
