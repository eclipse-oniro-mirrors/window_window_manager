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

#include "load_intention_event.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 10000;
    constexpr uint32_t RETRY_TIMES = 3;
}

static void *g_handle = nullptr;
using CreateAndEnableInputEventListenerFunc =  bool (*)(
    Ace::UIContent*, std::shared_ptr<AppExecFwk::EventHandler>, wptr<Window>);

bool LoadIntentionEvent(void)
{
    if (g_handle != nullptr) {
        TLOGW(WmsLogTag::WMS_EVENT, "Intention event has already exists.");
        return true;
    }
    uint32_t cnt = 0;
    const char* dlopenError = nullptr;
    do {
        cnt++;
        dlerror();
        g_handle = dlopen(INTENTION_EVENT_SO_PATH.c_str(), RTLD_LAZY);
        dlopenError = dlerror();
        if (dlopenError) {
            TLOGE(WmsLogTag::WMS_EVENT, "dlopen error: %{public}s", dlopenError);
        }
        TLOGI(WmsLogTag::WMS_EVENT, "dlopen %{public}s, retry cnt: %{public}d", INTENTION_EVENT_SO_PATH.c_str(), cnt);
        usleep(SLEEP_TIME_US);
    } while (!g_handle && cnt < RETRY_TIMES);
    return g_handle != nullptr;
}

void UnloadIntentionEvent(void)
{
    TLOGI(WmsLogTag::WMS_EVENT, "unload intention event.");
    if (g_handle != nullptr) {
        dlclose(g_handle);
        g_handle = nullptr;
    }
}

__attribute__((no_sanitize("cfi"))) bool EnableInputEventListener(
    Ace::UIContent* uiContent, std::shared_ptr<AppExecFwk::EventHandler> eventHandler, wptr<Window> window)
{
    if (g_handle == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "g_handle is nullptr");
        return false;
    }
    dlerror();
    CreateAndEnableInputEventListenerFunc func = reinterpret_cast<CreateAndEnableInputEventListenerFunc>(
        dlsym(g_handle, "CreateAndEnableInputEventListener"));
    const char* dlsymError = dlerror();
    if (dlsymError) {
        TLOGE(WmsLogTag::WMS_EVENT, "dlsym error: %{public}s", dlsymError);
        return false;
    }
    return func(uiContent, eventHandler, window);
}
} // namespace Rosen
} // namespace OHOS