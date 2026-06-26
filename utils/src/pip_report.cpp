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

#include <map>
#include <hisysevent.h>
#include "pip_report.h"
#include "window_manager_hilog.h"
#include "perform_reporter.h"
#include "singleton_container.h"

namespace OHOS {
namespace Rosen {
namespace {
const std::string PNAMEID = "windowManager";
const std::string PVERSION = "1.0";
}
WM_IMPLEMENT_SINGLE_INSTANCE(PiPReporter)

#define LOG_WHEN_ERROR(errCode)                                                               \
    do {                                                                                      \
        if ((errCode) != 0) {                                                                 \
            TLOGE(WmsLogTag::WMS_PIP, "Write HiSysEvent error, errCode:%{public}d", errCode); \
        }                                                                                     \
    } while (false)

static std::map<std::string, int32_t> CONTROL_ACTION_MAP = {
    {"playbackStateChanged", 0},
    {"nextVideo", 1},
    {"previousVideo", 2},
    {"hangUp", 3},
    {"micStateChanged", 4},
    {"videoStateChanged", 5},
    {"voiceStateChanged", 6},
    {"fastForward", 7},
    {"fastBackward", 8}
};
constexpr char EVENT_KEY_SOURCE[] = "SOURCE";
constexpr char EVENT_KEY_TEMPLATE_TYPE[] = "TEMPLATE_TYPE";
constexpr char EVENT_KEY_PNAMEID[] = "PNAMEID";
constexpr char EVENT_KEY_PVERSION[] = "PVERSIONID";
constexpr char EVENT_KEY_START_PACKAGE_NAME[] = "START_PACKAGE_NAME";
constexpr char EVENT_KEY_STOP_PACKAGE_NAME[] = "STOP_PACKAGE_NAME";
constexpr char EVENT_KEY_OPERATION_PACKAGE_NAME[] = "OPERATION_PACKAGE_NAME";
constexpr char EVENT_KEY_APP_PACKAGE_NAME[] = "APP_PACKAGE_NAME";
constexpr char EVENT_KEY_OPERATION_CODE[] = "OPERATION_CODE";
constexpr char EVENT_KEY_OPERATION_ERROR_REASON[] = "OPERATION_ERROR_REASON";
constexpr char EVENT_KEY_ACTION_EVENT[] = "ACTION_EVENT";
constexpr char EVENT_KEY_WINDOW_WIDTH[] = "WINDOW_WIDTH";
constexpr char EVENT_KEY_WINDOW_HEIGHT[] = "WINDOW_HEIGHT";
constexpr char TEMPLATE_TYPE[] = "TEMPLATE_TYPE";
constexpr char ORIGIN_CONTENT_TYPE[] = "ORIGIN_CONTENT_TYPE";

void PiPReporter::SetCurrentPackageName(const std::string& packageName)
{
    std::lock_guard<std::mutex> lock(packageNameMutex_);
    packageName_ = packageName;
}

std::string PiPReporter::GetPackageName() const
{
    std::lock_guard<std::mutex> lock(packageNameMutex_);
    return packageName_;
}

void PiPReporter::ReportPiPStartWindow(int32_t source, int32_t templateType,
                                       int32_t isSuccess, const std::string& errorReason)
{
    TLOGI(WmsLogTag::WMS_PIP, "Report start pip widow");
    if (source == 0) {
        TLOGI(WmsLogTag::WMS_PIP, "need not report start pip widow");
        return;
    }
    std::string eventName = "START_PIP";
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::MULTIWINDOW_UE, eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        EVENT_KEY_PNAMEID, PNAMEID,
        EVENT_KEY_PVERSION, PVERSION,
        EVENT_KEY_SOURCE, source,
        EVENT_KEY_TEMPLATE_TYPE, templateType,
        EVENT_KEY_START_PACKAGE_NAME, GetPackageName(),
        EVENT_KEY_OPERATION_CODE, isSuccess,
        EVENT_KEY_OPERATION_ERROR_REASON, errorReason);
    LOG_WHEN_ERROR(ret);
}

void PiPReporter::ReportPiPStopWindow(int32_t source, int32_t templateType,
                                      int32_t isSuccess, const std::string& errorReason)
{
    TLOGI(WmsLogTag::WMS_PIP, "Report stop pip widow");
    if (source == 0) {
        TLOGI(WmsLogTag::WMS_PIP, "need not report stop pip widow");
        return;
    }
    std::string eventName = "STOP_PIP";
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::MULTIWINDOW_UE, eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        EVENT_KEY_PNAMEID, PNAMEID,
        EVENT_KEY_PVERSION, PVERSION,
        EVENT_KEY_SOURCE, source,
        EVENT_KEY_TEMPLATE_TYPE, templateType,
        EVENT_KEY_STOP_PACKAGE_NAME, GetPackageName(),
        EVENT_KEY_OPERATION_CODE, isSuccess,
        EVENT_KEY_OPERATION_ERROR_REASON, errorReason);
    LOG_WHEN_ERROR(ret);
}

void PiPReporter::ReportPiPActionEvent(int32_t templateType, const std::string& actionEvent)
{
    TLOGI(WmsLogTag::WMS_PIP, "Report pip widow action event");
    std::string eventName = "CONTROL_PANNEL_ACTION_EVENT";
    if (CONTROL_ACTION_MAP.find(actionEvent) == CONTROL_ACTION_MAP.end()) {
        TLOGE(WmsLogTag::WMS_PIP, "actionEvent %{public}s not found", actionEvent.c_str());
        return;
    }
    int32_t currentAction = CONTROL_ACTION_MAP[actionEvent];
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::MULTIWINDOW_UE, eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        EVENT_KEY_PNAMEID, PNAMEID,
        EVENT_KEY_PVERSION, PVERSION,
        EVENT_KEY_TEMPLATE_TYPE, templateType,
        EVENT_KEY_ACTION_EVENT, currentAction,
        EVENT_KEY_OPERATION_PACKAGE_NAME, GetPackageName());
    LOG_WHEN_ERROR(ret);
}

void PiPReporter::ReportPiPControlEvent(int32_t templateType, PiPControlType controlType)
{
    TLOGI(WmsLogTag::WMS_PIP, "templateType:%{public}d, controlType:%{public}d", templateType, controlType);
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::MULTIWINDOW_UE, "CONTROL_CONTROL_EVENT",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        EVENT_KEY_PNAMEID, PNAMEID,
        EVENT_KEY_PVERSION, PVERSION,
        EVENT_KEY_TEMPLATE_TYPE, templateType,
        EVENT_KEY_ACTION_EVENT, static_cast<uint32_t>(controlType),
        EVENT_KEY_OPERATION_PACKAGE_NAME, GetPackageName());
    LOG_WHEN_ERROR(ret);
}

void PiPReporter::ReportPiPRatio(int32_t windowWidth, int32_t windowHeight)
{
    TLOGI(WmsLogTag::WMS_PIP, "Report pip widow ratio");
    std::string eventName = "UPDATE_PIP_RATIO";
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::MULTIWINDOW_UE, eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        EVENT_KEY_PNAMEID, PNAMEID,
        EVENT_KEY_PVERSION, PVERSION,
        EVENT_KEY_WINDOW_WIDTH, windowWidth,
        EVENT_KEY_WINDOW_HEIGHT, windowHeight,
        EVENT_KEY_OPERATION_PACKAGE_NAME, GetPackageName());
    LOG_WHEN_ERROR(ret);
}

void PiPReporter::ReportPiPRestore()
{
    TLOGI(WmsLogTag::WMS_PIP, "Report pip widow restore");
    std::string eventName = "RESOTRE_PIP";
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::MULTIWINDOW_UE, eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        EVENT_KEY_PNAMEID, PNAMEID,
        EVENT_KEY_PVERSION, PVERSION,
        EVENT_KEY_OPERATION_PACKAGE_NAME, GetPackageName());
    LOG_WHEN_ERROR(ret);
}

void PiPReporter::ReportPiPUpdateContent(int32_t originType, int32_t templateType,
    int32_t isSuccess, const std::string &errorReason)
{
    TLOGI(WmsLogTag::WMS_PIP, "Report pip widow update typeNode");
    std::string eventName = "UPDATE_CONTENT_NODE";
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::MULTIWINDOW_UE, eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        EVENT_KEY_PNAMEID, PNAMEID,
        EVENT_KEY_PVERSION, PVERSION,
        ORIGIN_CONTENT_TYPE, originType,
        TEMPLATE_TYPE, templateType,
        EVENT_KEY_OPERATION_CODE, isSuccess,
        EVENT_KEY_OPERATION_ERROR_REASON, errorReason,
        EVENT_KEY_APP_PACKAGE_NAME, GetPackageName());
    LOG_WHEN_ERROR(ret);
}

void PiPReporter::ReportWindowException(const std::string& funName, uint32_t errCode,
    const std::string& errorReason) const
{
    std::ostringstream oss;
    TLOGI(WmsLogTag::WMS_PIP, "Report pip widow exception");
    oss << "ModuleName: PIP" << ", FunName: " << funName << ", ErrCode: " <<
        errCode << ", ErrorReason: " << errorReason;
    auto type = WindowDFXHelperType::WINDOW_PIP_CHECK;
    WindowInfoReporter::GetInstance().ReportWindowException(static_cast<int32_t>(type), getpid(), oss.str());
}
} // namespace Rosen
} // namespace OHOS
