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

#include "product_ext_wrapper.h"
#include <dlfcn.h>
#include <unistd.h>
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace DMS {
namespace {
#if (defined(__aarch64__) || defined(__x86_64__))
const char* EXT_PLUGIN_SO_PATH = "/system/lib64/libdm_extension.z.so";
#else
const char* EXT_PLUGIN_SO_PATH = "/system/lib/libdm_extension.z.so";
#endif
constexpr uint32_t SLEEP_TIME_US = 10000;
constexpr uint32_t SLEEP_TIME_FUNC_US = 100000;
constexpr uint32_t MAX_LOAD_RETRY_TIMES = 3;
}  // namespace

void* ProductExtWrapper::dmsExtHandle_ = nullptr;
std::mutex ProductExtWrapper::dmsExtHandleMutex_;

void ProductExtWrapper::InitProductExtWrapper()
{
    TryLoadDmsExtension();
    if (dmsExtHandle_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "load dms extension failed.");
    }
}

void* ProductExtWrapper::GetHandleByName(const std::string& getFuncName)
{
    void* funcHandle = nullptr;
    uint32_t cnt = 0;
    do {
        cnt++;
        TryReloadDmsExtension();
        if (dmsExtHandle_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "dmsExtHandle is null, reload so");
            return nullptr;
        }
        funcHandle = dlsym(dmsExtHandle_, getFuncName.c_str());
        auto dlsymError = dlerror();
        if (funcHandle == nullptr) {
            TLOGE(WmsLogTag::DMS, "get symbol failed, error: %{public}s", dlsymError);
            usleep(SLEEP_TIME_FUNC_US);
        }
    } while (funcHandle == nullptr && cnt < MAX_LOAD_RETRY_TIMES);
    return funcHandle;
}

void ProductExtWrapper::TryLoadDmsExtension()
{
    TLOGI(WmsLogTag::DMS, "load DmsExtension start");
    uint32_t cnt = 0;
    do {
        cnt++;
        dmsExtHandle_ = dlopen(EXT_PLUGIN_SO_PATH, RTLD_LAZY);
        TLOGI(WmsLogTag::DMS, "dlopen %{public}s, retry cnt: %{public}d", EXT_PLUGIN_SO_PATH, cnt);
        if (dmsExtHandle_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "dlopen failed: %{public}s", dlerror());
            usleep(SLEEP_TIME_US);
        }
    } while (dmsExtHandle_ == nullptr && cnt < MAX_LOAD_RETRY_TIMES);
}

void ProductExtWrapper::TryReloadDmsExtension()
{
    std::lock_guard<std::mutex> lock(dmsExtHandleMutex_);
    if (dmsExtHandle_ == nullptr) {
        TryLoadDmsExtension();
    }
}
}  // namespace DMS
}  // namespace Rosen
}  // namespace OHOS