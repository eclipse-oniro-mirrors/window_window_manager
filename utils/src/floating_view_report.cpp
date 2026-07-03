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

#include <hisysevent.h>
#include "floating_view_report.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr char PNAMEID[] = "SCB_FLOATING_VIEW";
constexpr char PVERSION[] = "1.0";

constexpr char EVENT_KEY_PNAMEID[] = "PNAMEID";
constexpr char EVENT_KEY_PVERSION[] = "PVERSION";
constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_CONTROLLER_ID[] = "CONTROLLER_ID";
constexpr char EVENT_KEY_TEMPLATE_TYPE[] = "TEMPLATE_TYPE";
constexpr char EVENT_KEY_BIND_FLOATING_BALL[] = "BIND_FLOATING_BALL";
constexpr char EVENT_KEY_ERROR_REASON[] = "ERROR_REASON";
constexpr char EVENT_KEY_WIDTH[] = "WIDTH";
constexpr char EVENT_KEY_HEIGHT[] = "HEIGHT";
constexpr char EVENT_KEY_VISIBLE_IN_APP[] = "VISIBLE_IN_APP";
constexpr char EVENT_KEY_IS_LOAD_UI[] = "IS_LOAD_UI";
}

using HiSysEvent = OHOS::HiviewDFX::HiSysEvent;

WM_IMPLEMENT_SINGLE_INSTANCE(FloatingViewReporter)

void FloatingViewReporter::SetCurrentPackageName(const std::string& packageName)
{
    std::lock_guard<std::mutex> lock(packageNameMutex_);
    packageName_ = packageName;
}

std::string FloatingViewReporter::GetPackageName() const
{
    std::lock_guard<std::mutex> lock(packageNameMutex_);
    return packageName_;
}

template<typename... ExtraArgs>
void FloatingViewReporter::WriteHiSysEvent(const std::string& eventName,
    const FvBaseEventParams& base, ExtraArgs &&... extraArgs) const
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "WriteHiSysEvent, eventName:%{public}s, controllerId:%{public}s",
        eventName.c_str(), base.controllerId.c_str());
    int32_t ret = HiSysEventWrite(
        HiSysEvent::Domain::MULTIWINDOW_UE, eventName,
        HiSysEvent::EventType::BEHAVIOR,
        EVENT_KEY_PNAMEID, PNAMEID,
        EVENT_KEY_PVERSION, PVERSION,
        EVENT_KEY_PACKAGE_NAME, GetPackageName(),
        EVENT_KEY_CONTROLLER_ID, base.controllerId,
        EVENT_KEY_TEMPLATE_TYPE, base.templateType,
        EVENT_KEY_BIND_FLOATING_BALL, base.bindFloatingBall,
        std::forward<ExtraArgs>(extraArgs)...,
        EVENT_KEY_ERROR_REASON, base.errorReason);
    if (ret != 0) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Write HiSysEvent error, eventName:%{public}s, errCode:%{public}d",
            eventName.c_str(), ret);
    }
}

void FloatingViewReporter::ReportStartEvent(const FvStartEventParams& params)
{
    WriteHiSysEvent(FloatingViewEvent::EVENT_KEY_START, params,
        EVENT_KEY_WIDTH, params.width,
        EVENT_KEY_HEIGHT, params.height,
        EVENT_KEY_VISIBLE_IN_APP, params.visibleInApp,
        EVENT_KEY_IS_LOAD_UI, params.isLoadUi);
}

void FloatingViewReporter::ReportRestoreEvent(const FvRestoreEventParams& params)
{
    WriteHiSysEvent(FloatingViewEvent::EVENT_KEY_RESTORE, params);
}

void FloatingViewReporter::ReportSetUIContentEvent(const FvSetUIContentEventParams& params)
{
    WriteHiSysEvent(FloatingViewEvent::EVENT_KEY_SET_UI_CONTENT, params);
}

void FloatingViewReporter::ReportSetSizeEvent(const FvSetSizeEventParams& params)
{
    WriteHiSysEvent(FloatingViewEvent::EVENT_KEY_SET_SIZE, params,
        EVENT_KEY_WIDTH, params.width,
        EVENT_KEY_HEIGHT, params.height);
}

void FloatingViewReporter::ReportSetVisibilityEvent(const FvSetVisibilityEventParams& params)
{
    WriteHiSysEvent(FloatingViewEvent::EVENT_KEY_SET_VISIBILITY, params,
        EVENT_KEY_VISIBLE_IN_APP, params.visibleInApp);
}

} // namespace Rosen
} // namespace OHOS