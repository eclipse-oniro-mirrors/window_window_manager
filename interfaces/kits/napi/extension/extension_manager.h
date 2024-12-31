/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_EXTENSION_MANAGER_H
#define OHOS_ROSEN_EXTENSION_MANAGER_H

#include "native_engine/native_engine.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_value.h"

namespace OHOS {
namespace Rosen {
class ExtensionManager {
public:
    static void Finalizer(napi_env env, void* data, void* hint);
};

napi_value ExtensionModuleInit(napi_env env, napi_value exportObj);
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_EXTENSION_MANAGER_H