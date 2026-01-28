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

#include "sensor_plugin.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 10000;
}

#if (defined(__aarch64__) || defined(__x86_64__))
const std::string PLUGIN_SO_PATH = "/system/lib64/platformsdk/libmotion_agent.z.so";
#else
const std::string PLUGIN_SO_PATH = "/system/lib/platformsdk/libmotion_agent.z.so";
#endif

static void *g_handle = nullptr;

bool LoadMotionSensor(void)
{
    if (g_handle != nullptr) {
        TLOGW(WmsLogTag::DMS, "motion plugin has already exits.");
        return true;
    }
    int32_t cnt = 0;
    int32_t retryTimes = 3;
    do {
        cnt++;
        g_handle = dlopen(PLUGIN_SO_PATH.c_str(), RTLD_LAZY);
        TLOGI(WmsLogTag::DMS, "dlopen %{public}s, retry cnt: %{public}d", PLUGIN_SO_PATH.c_str(), cnt);
        usleep(SLEEP_TIME_US);
    } while (!g_handle && cnt < retryTimes);
    return g_handle != nullptr;
}

void UnloadMotionSensor(void)
{
    TLOGI(WmsLogTag::DMS, "unload motion plugin.");
    if (g_handle != nullptr) {
        dlclose(g_handle);
        g_handle = nullptr;
    }
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
    MotionSubscribeCallbackPtr func = (MotionSubscribeCallbackPtr)(dlsym(g_handle, "MotionSubscribeCallback"));
    const char* dlsymError = dlerror();
    if  (dlsymError) {
        TLOGE(WmsLogTag::DMS, "dlsym error: %{public}s", dlsymError);
        return false;
    }
    return func(motionType, callback);
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
    MotionUnsubscribeCallbackPtr func =
        (MotionUnsubscribeCallbackPtr)(dlsym(g_handle, "MotionUnsubscribeCallback"));
    const char* dlsymError = dlerror();
    if  (dlsymError) {
        TLOGE(WmsLogTag::DMS, "dlsym error: %{public}s", dlsymError);
        return false;
    }
    return func(motionType, callback);
}
}
}