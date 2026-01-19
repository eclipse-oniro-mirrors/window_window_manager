/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef LOAD_MMI_CLIENT_ADAPTER_H
#define LOAD_MMI_CLIENT_ADAPTER_H

#include <dlfcn.h>
#include <memory>
#include <string>
#include <unistd.h>

#include "marshalling_helper.h"
#include "window_manager_hilog.h"

namespace OHOS::MMI {
class PointerEvent;
}

namespace OHOS {
namespace Rosen {
#if (defined(__aarch64__) || defined(__x86_64__))
const std::string MMI_CLIENT_ADAPTER_SO_PATH = "/system/lib64/libmmi_client_adapter.z.so";
#else
const std::string MMI_CLIENT_ADAPTER_SO_PATH = "/system/lib/libmmi_client_adapter.z.so";
#endif

bool LoadMMIClientAdapter(void);
void UnloadMMIClientAdapter(void);
bool IsHoverDown(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
bool PointerEventWriteToParcel(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, Parcel& data);
} // namespace Rosen
} // namespace OHOS
#endif /* LOAD_MMI_CLIENT_ADAPTER_H */