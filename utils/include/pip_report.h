/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef WINDOW_WINDOW_MANAGER_PIP_REPORT_H
#define WINDOW_WINDOW_MANAGER_PIP_REPORT_H

#include <mutex>
#include "wm_single_instance.h"
#include "wm_common.h"

namespace OHOS::Rosen {
class PiPReporter {
WM_DECLARE_SINGLE_INSTANCE(PiPReporter);

public:
    void SetCurrentPackageName(const std::string& packageName);
    void ReportPiPStartWindow(int32_t source, int32_t templateType, int32_t isSuccess, const std::string& errorReason);
    void ReportPiPStopWindow(int32_t source, int32_t templateType, int32_t isSuccess, const std::string& errorReason);
    void ReportPiPActionEvent(int32_t templateType, const std::string& actionEvent);
    void ReportPiPControlEvent(int32_t templateType, PiPControlType controlType);
    void ReportPiPRatio(int32_t windowWidth, int32_t windowHeight);
    void ReportPiPRestore();
    void ReportPiPUpdateContent(int32_t originType, int32_t templateType, int32_t isSuccess,
        const std::string &errorReason);
    void ReportWindowException(const std::string& funName, uint32_t errCode, const std::string& errorReason) const;
private:
    std::string GetPackageName() const;
    std::string packageName_ = "";
    mutable std::mutex packageNameMutex_;
};
} // namespace OHOS::Rosen

#endif //WINDOW_WINDOW_MANAGER_PIP_REPORT_H
