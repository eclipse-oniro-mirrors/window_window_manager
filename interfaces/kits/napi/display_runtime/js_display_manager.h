/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_DISPLAY_MANAGER_H
#define OHOS_JS_DISPLAY_MANAGER_H

#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "dm_common.h"

namespace OHOS {
namespace Rosen {
    std::map<DmErrorCode, std::string> errCodeMap = {
        {DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT, "Capability not supported."},
        {DmErrorCode::DM_ERROR_SYSTEM_INNORMAL, "This display manager service works abnormally."},
        {DmErrorCode::DM_ERROR_ILLEGAL_PARAM, "Parameter error. Possible cause: 1. Invalid parameter range."}
};

napi_value JsDisplayManagerInit(napi_env env, napi_value exportObj);
}  // namespace Rosen
}  // namespace OHOS

#endif
