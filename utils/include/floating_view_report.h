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

#ifndef OHOS_FLOATING_VIEW_REPORT_H
#define OHOS_FLOATING_VIEW_REPORT_H

#include <mutex>
#include <string>
#include "wm_single_instance.h"

namespace OHOS::Rosen {
namespace FloatingViewEvent {
constexpr char EVENT_KEY_START[] = "START_FLOAT_VIEW";
constexpr char EVENT_KEY_RESTORE[] = "RESTORE_FLOAT_VIEW_MAIN_WINDOW";
constexpr char EVENT_KEY_SET_UI_CONTENT[] = "SET_FLOAT_VIEW_UI_CONTENT";
constexpr char EVENT_KEY_SET_SIZE[] = "SET_FLOAT_VIEW_SIZE";
constexpr char EVENT_KEY_SET_VISIBILITY[] = "SET_FLOAT_VIEW_VISIBILITY_IN_APP";
}

struct FvBaseEventParams {
    std::string controllerId;
    uint32_t templateType = 0;
    bool bindFloatingBall = false;
    std::string errorReason;
};

struct FvStartEventParams : FvBaseEventParams {
    uint32_t width = 0;
    uint32_t height = 0;
    bool visibleInApp = false;
    bool isLoadUi = false;
};

struct FvRestoreEventParams : FvBaseEventParams {};

struct FvSetUIContentEventParams : FvBaseEventParams {};

struct FvSetSizeEventParams : FvBaseEventParams {
    uint32_t width = 0;
    uint32_t height = 0;
};

struct FvSetVisibilityEventParams : FvBaseEventParams {
    bool visibleInApp = false;
};

class FloatingViewReporter {
WM_DECLARE_SINGLE_INSTANCE(FloatingViewReporter);

public:
    void SetCurrentPackageName(const std::string& packageName);
    void ReportStartEvent(const FvStartEventParams& params);
    void ReportRestoreEvent(const FvRestoreEventParams& params);
    void ReportSetUIContentEvent(const FvSetUIContentEventParams& params);
    void ReportSetSizeEvent(const FvSetSizeEventParams& params);
    void ReportSetVisibilityEvent(const FvSetVisibilityEventParams& params);

private:
    std::string GetPackageName() const;
    template<typename... ExtraArgs>
    void WriteHiSysEvent(const std::string& eventName, const FvBaseEventParams& base,
                         ExtraArgs &&... extraArgs) const;
    std::string packageName_ = "";
    mutable std::mutex packageNameMutex_;
};
} // namespace OHOS::Rosen

#endif // OHOS_FLOATING_VIEW_REPORT_H