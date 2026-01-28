/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_WS_INTENTION_EVENT_ENTRANCE_LOG
#define OHOS_WS_INTENTION_EVENT_ENTRANCE_LOG

#include <cinttypes>
#include <functional>
#include <future>
#include <string>
#include <sstream>

#include "hilog/log.h"

#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
class InnerFunctionTracer {
public:
    using HilogFunc = std::function<int(const char *)>;

public:
    InnerFunctionTracer(HilogFunc logfn, const char* tag, LogLevel level)
        : logfn_ { logfn }, tag_ { tag }, level_ { level }
    {
        if (HiLogIsLoggable(OHOS::Rosen::HILOG_DOMAIN_WINDOW, tag_, level_)) {
            if (logfn_ != nullptr) {
                logfn_("enter");
            }
        }
    }
    ~InnerFunctionTracer()
    {
        if (HiLogIsLoggable(OHOS::Rosen::HILOG_DOMAIN_WINDOW, tag_, level_)) {
            if (logfn_ != nullptr) {
                logfn_("leave");
            }
        }
    }
    
private:
    HilogFunc logfn_ { nullptr };
    const char* tag_ { nullptr };
    LogLevel level_ { LOG_LEVEL_MIN };
};

} // namespace Rosen
}
#endif // OHOS_WS_INTENTION_EVENT_ENTRANCE_LOG
