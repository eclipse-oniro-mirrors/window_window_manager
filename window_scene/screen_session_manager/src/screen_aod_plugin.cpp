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
#include "screen_aod_plugin.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 10000;
    constexpr uint32_t RETRY_TIMES = 3;
}

static void *g_handle = nullptr;
using IsInAodFunc = bool (*)();
using StopAodFunc = bool (*)(int32_t);
static IsInAodFunc g_isInAodFunc = nullptr;
static StopAodFunc g_stopAodFunc = nullptr;

bool LoadAodLib(void)
{
    if (g_handle != nullptr) {
        TLOGW(WmsLogTag::DMS, "aod plugin has already exits.");
        return true;
    }
    int32_t cnt = 0;
    const char* dlopenError = nullptr;
    do {
        cnt++;
        g_handle = dlopen(PLUGIN_AOD_SO_PATH.c_str(), RTLD_LAZY);
        dlopenError = dlerror();
        if (dlopenError) {
            TLOGE(WmsLogTag::DMS, "dlopen error: %{public}s", dlopenError);
        }
        TLOGI(WmsLogTag::DMS, "dlopen %{public}s, retry cnt: %{public}d", PLUGIN_AOD_SO_PATH.c_str(), cnt);
        usleep(SLEEP_TIME_US);
    } while (!g_handle && cnt < RETRY_TIMES);
    return g_handle != nullptr;
}

void UnloadAodLib(void)
{
    TLOGI(WmsLogTag::DMS, "unload aod plugin.");
    if (g_handle != nullptr) {
        dlclose(g_handle);
        g_handle = nullptr;
    }
    g_isInAodFunc = nullptr;
    g_stopAodFunc = nullptr;
}

__attribute__((no_sanitize("cfi"))) bool IsInAod()
{
    if (g_handle == nullptr) {
        TLOGE(WmsLogTag::DMS, "g_handle is nullptr");
        return false;
    }
    if (g_isInAodFunc == nullptr) {
        int32_t cnt = 0;
        const char* dlsymError = nullptr;
        do {
            cnt++;
            g_isInAodFunc = reinterpret_cast<IsInAodFunc>(dlsym(g_handle, "IsInAod"));
            dlsymError = dlerror();
            if (dlsymError) {
                TLOGE(WmsLogTag::DMS, "dlsym error: %{public}s", dlsymError);
                usleep(SLEEP_TIME_US);
            }
            TLOGI(WmsLogTag::DMS, "dlsym %{public}s, retry cnt: %{public}d", "IsInAod", cnt);
        } while (!g_isInAodFunc && cnt < RETRY_TIMES);
    }
    if (g_isInAodFunc == nullptr) {
        return false;
    } else {
        return g_isInAodFunc();
    }
}

__attribute__((no_sanitize("cfi"))) bool StopAod(int32_t status)
{
    if (g_handle == nullptr) {
        TLOGE(WmsLogTag::DMS, "g_handle is nullptr");
        return false;
    }
    if (g_stopAodFunc == nullptr) {
        int32_t cnt = 0;
        const char* dlsymError = nullptr;
        do {
            cnt++;
            g_stopAodFunc = reinterpret_cast<StopAodFunc>(dlsym(g_handle, "StopAod"));
            dlsymError = dlerror();
            if (dlsymError) {
                TLOGE(WmsLogTag::DMS, "dlsym error: %{public}s", dlsymError);
                usleep(SLEEP_TIME_US);
            }
            TLOGI(WmsLogTag::DMS, "dlsym %{public}s, retry cnt: %{public}d", "StopAod", cnt);
        } while (!g_stopAodFunc && cnt < RETRY_TIMES);
    }
    if (g_stopAodFunc == nullptr) {
        return false;
    } else {
        return g_stopAodFunc(status);
    }
}
}
}