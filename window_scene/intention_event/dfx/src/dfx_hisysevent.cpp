/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dfx_hisysevent.h"
#include "entrance_log.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "DfxHisysevent" };
} // namespace

void DfxHisysevent::ApplicationBlockInput(int32_t eventId, int32_t pid,
    const std::string& bundleName, int32_t persistentId)
{
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "APPLICATION_BLOCK_INPUT",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "EVENT_ID", eventId,
        "PID", pid,
        "BUNDLE_NAME", bundleName,
        "PERSISTENT_ID", persistentId,
        "MSG", "User input does not respond normally, report by sceneBoard.");
    if (ret != 0) {
        WLOGFE("Write HiSysEvent error, ret:%{public}d, eventId:%{public}d, pid:%{public}d, bundleName:%{public}s, "
               "persistentId:%{public}d", ret, eventId, pid, bundleName.c_str(), persistentId);
    }
}
} // namespace Rosen
} // namespace OHOS

