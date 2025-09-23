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

#ifndef SCREEN_AOD_PLUGIN_H
#define SCREEN_AOD_PLUGIN_H

#include <string>

#include <dlfcn.h>
#include <unistd.h>
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

#if (defined(__aarch64__) || defined(__x86_64__))
const std::string PLUGIN_AOD_SO_PATH = "/system/lib64/libaod_native.z.so";
#else
const std::string PLUGIN_AOD_SO_PATH = "/system/lib/libaod_native.z.so";
#endif

bool LoadAodLib(void);
void UnloadAodLib(void);
bool IsInAod();
bool StopAod(int32_t status);
}
}
#endif /* SCREEN_AOD_PLUGIN_H */