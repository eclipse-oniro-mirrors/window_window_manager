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

#include "load_mmi_client_adapter.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 10000;
    constexpr uint32_t RETRY_TIMES = 3;
}

using IsHoverDownFunc =  bool (*)(const std::shared_ptr<MMI::PointerEvent>&);
using PointerEventWriteToParcelFunc =  bool (*)(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, Parcel& data);

static void *g_handle = nullptr;
static IsHoverDownFunc g_isHoverDownFunc = nullptr;
static PointerEventWriteToParcelFunc g_pointerEventWriteToParcelFunc = nullptr;

bool LoadMMIClientAdapter(void)
{
    if (g_handle != nullptr) {
        TLOGD(WmsLogTag::WMS_EVENT, "MMI client adapter has already exists.");
        return true;
    }
    uint32_t cnt = 0;
    const char* dlopenError = nullptr;
    do {
        cnt++;
        dlerror();
        g_handle = dlopen(MMI_CLIENT_ADAPTER_SO_PATH.c_str(), RTLD_LAZY);
        dlopenError = dlerror();
        if (dlopenError) {
            TLOGE(WmsLogTag::WMS_EVENT, "dlopen error: %{public}s", dlopenError);
        }
        TLOGI(WmsLogTag::WMS_EVENT, "dlopen %{public}s, retry cnt: %{public}d",
            MMI_CLIENT_ADAPTER_SO_PATH.c_str(), cnt);
        usleep(SLEEP_TIME_US);
    } while (!g_handle && cnt < RETRY_TIMES);
    return g_handle != nullptr;
}

void UnloadMMIClientAdapter(void)
{
    TLOGI(WmsLogTag::WMS_EVENT, "unload MMI client adapter.");
    if (g_handle != nullptr) {
        dlclose(g_handle);
        g_handle = nullptr;
    }
    g_isHoverDownFunc = nullptr;
    g_pointerEventWriteToParcelFunc = nullptr;
}

__attribute__((no_sanitize("cfi"))) bool IsHoverDown(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (g_handle == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "g_handle is nullptr");
        return false;
    }
    if (g_isHoverDownFunc == nullptr) {
        dlerror();
        g_isHoverDownFunc = reinterpret_cast<IsHoverDownFunc>(dlsym(g_handle, "IsHoverDown"));
        const char* dlsymError = dlerror();
        if (dlsymError) {
            TLOGE(WmsLogTag::WMS_EVENT, "dlsym error: %{public}s", dlsymError);
            return false;
        }
    }
    if (g_isHoverDownFunc == nullptr) {
        return false;
    }
    return g_isHoverDownFunc(pointerEvent);
}

__attribute__((no_sanitize("cfi"))) bool PointerEventWriteToParcel(
    const std::shared_ptr<MMI::PointerEvent>& pointerEvent, Parcel& data)
{
    if (g_handle == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "g_handle is nullptr");
        return false;
    }
    if (g_pointerEventWriteToParcelFunc == nullptr) {
        dlerror();
        g_pointerEventWriteToParcelFunc = reinterpret_cast<PointerEventWriteToParcelFunc>(
            dlsym(g_handle, "PointerEventWriteToParcel"));
        const char* dlsymError = dlerror();
        if (dlsymError) {
            TLOGE(WmsLogTag::WMS_EVENT, "dlsym error: %{public}s", dlsymError);
            return false;
        }
    }
    if (g_pointerEventWriteToParcelFunc == nullptr) {
        return false;
    }
    return g_pointerEventWriteToParcelFunc(pointerEvent, data);
}
} // namespace Rosen
} // namespace OHOS