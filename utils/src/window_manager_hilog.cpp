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

#include "window_manager_hilog.h"
#include <chrono>

namespace OHOS {
namespace Rosen {
const char* g_domainContents[static_cast<uint32_t>(WmsLogTag::END)] = {
    "WMS",
    "DMS",
    "WMSMain",
    "WMSSub",
    "WMSScb",
    "WMSDialog",
    "WMSSystem",
    "WMSFocus",
    "WMSLayout",
    "WMSImms",
    "WMSLife",
    "WMSKeyboard",
    "WMSEvent",
    "WMSUiext",
    "WMSPiP",
    "WMSRecover",
    "WMSMultiUser",
    "WMSToast",
    "WMSMultiWindow",
    "InputKeyFlow",
    "WMSPipeLine",
    "WMSHierarchy",
    "WMSPattern",
    "WMSAttribute",
    "WMSPc",
    "WMSDecor",
    "WMSLayoutPc",
    "WMSStartupPage",
    "WMSCompat",
    "WMSRotation",
    "WMSAnimation",
};

TLogInfo GetTLogInfo(WmsLogTag tag)
{
    uint32_t domain = HILOG_DOMAIN_WINDOW + static_cast<uint32_t>(tag);
    const char* content = (tag >= WmsLogTag::DEFAULT && tag < WmsLogTag::END) ?
        g_domainContents[static_cast<uint32_t>(tag)] : "";
    return {domain, content};
}

bool WinPrintLimit(const WinPrintLimitConfig& config, WinPrintLimitState& state)
{
    auto info = GetTLogInfo(config.logTag);
    auto now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    auto duration = now - state.last;
    if (duration.count() >= config.timeIntervals) {
        state.last = now;
        uint32_t supressedCnt = state.supressed.load();
        state.supressed.store(0);
        state.printCount.store(1);
        if (supressedCnt != 0) {
            HiLogPrint(LOG_CORE, config.logLevel, info.domain, info.content,
                "%{public}s log suppressed cnt %{public}u", config.functionName, supressedCnt);
        }
        return true;
    } else {
        int count = state.printCount.load();
        if (count < config.printFrequency) {
            state.printCount.store(count + 1);
            return true;
        } else {
            state.supressed.fetch_add(1);
            return false;
        }
    }
}
} // namespace OHOS
}