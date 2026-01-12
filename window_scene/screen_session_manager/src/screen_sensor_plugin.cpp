/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "screen_sensor_plugin.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 10000;
    constexpr uint32_t RETRY_TIMES = 3;
}

static void *g_handle = nullptr;
static MotionSubscribeCallbackPtr g_motionSubscribeCallbackPtr = nullptr;
static MotionUnsubscribeCallbackPtr g_motionUnsubscribeCallbackPtr = nullptr;

bool LoadMotionSensor(void)
{
    if (g_handle != nullptr) {
        TLOGW(WmsLogTag::DMS, "motion plugin has already exits.");
        return true;
    }
    int32_t cnt = 0;
    const char* dlopenError = nullptr;
    do {
        cnt++;
        g_handle = dlopen(PLUGIN_SO_PATH.c_str(), RTLD_LAZY);
        dlopenError = dlerror();
        if (dlopenError) {
            TLOGE(WmsLogTag::DMS, "dlopen error: %{public}s", dlopenError);
        }
        TLOGI(WmsLogTag::DMS, "dlopen %{public}s, retry cnt: %{public}d", PLUGIN_SO_PATH.c_str(), cnt);
        usleep(SLEEP_TIME_US);
    } while (!g_handle && cnt < RETRY_TIMES);
    return g_handle != nullptr;
}

void UnloadMotionSensor(void)
{
    TLOGI(WmsLogTag::DMS, "unload motion plugin.");
    if (g_handle != nullptr) {
        dlclose(g_handle);
        g_handle = nullptr;
    }
    g_motionSubscribeCallbackPtr = nullptr;
    g_motionUnsubscribeCallbackPtr = nullptr;
}

__attribute__((no_sanitize("cfi"))) bool SubscribeCallback(int32_t motionType, OnMotionChangedPtr callback)
{
    if (callback == nullptr) {
        TLOGE(WmsLogTag::DMS, "callback is nullptr");
        return false;
    }
    if (g_handle == nullptr) {
        TLOGE(WmsLogTag::DMS, "g_handle is nullptr");
        return false;
    }
    if (g_motionSubscribeCallbackPtr == nullptr) {
        int32_t cnt = 0;
        const char* dlsymError = nullptr;
        do {
            cnt++;
            g_motionSubscribeCallbackPtr = reinterpret_cast<MotionSubscribeCallbackPtr>(dlsym(
                g_handle, "MotionSubscribeCallback"));
            dlsymError = dlerror();
            if (dlsymError) {
                TLOGE(WmsLogTag::DMS, "dlsym error: %{public}s", dlsymError);
                usleep(SLEEP_TIME_US);
            }
            TLOGI(WmsLogTag::DMS, "dlsym %{public}s, retry cnt: %{public}d", "MotionSubscribeCallback", cnt);
        } while (!g_motionSubscribeCallbackPtr && cnt < RETRY_TIMES);
    }
    if (g_motionSubscribeCallbackPtr == nullptr) {
        return false;
    } else {
        return g_motionSubscribeCallbackPtr(motionType, callback);
    }
}

__attribute__((no_sanitize("cfi"))) bool UnsubscribeCallback(int32_t motionType, OnMotionChangedPtr callback)
{
    if (callback == nullptr) {
        TLOGE(WmsLogTag::DMS, "callback is nullptr");
        return false;
    }
    if (g_handle == nullptr) {
        TLOGE(WmsLogTag::DMS, "g_handle is nullptr");
        return false;
    }
    if (g_motionUnsubscribeCallbackPtr == nullptr) {
        int32_t cnt = 0;
        const char* dlsymError = nullptr;
        do {
            cnt++;
            g_motionUnsubscribeCallbackPtr = reinterpret_cast<MotionUnsubscribeCallbackPtr>(dlsym(
                g_handle, "MotionUnsubscribeCallback"));
            dlsymError = dlerror();
            if (dlsymError) {
                TLOGE(WmsLogTag::DMS, "dlsym error: %{public}s", dlsymError);
                usleep(SLEEP_TIME_US);
            }
            TLOGI(WmsLogTag::DMS, "dlsym %{public}s, retry cnt: %{public}d", "MotionUnsubscribeCallback", cnt);
        } while (!g_motionUnsubscribeCallbackPtr && cnt < RETRY_TIMES);
    }
    if (g_motionUnsubscribeCallbackPtr == nullptr) {
        return false;
    } else {
        return g_motionUnsubscribeCallbackPtr(motionType, callback);
    }
}
}
}