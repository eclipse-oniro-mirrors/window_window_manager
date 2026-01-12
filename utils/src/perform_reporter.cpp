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

#include "perform_reporter.h"

#include <hisysevent.h>

#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "PerformReporter"};

const std::map<KeyboardLifeCycleException, std::string> KEYBOARD_LIFE_CYCLE_EXCEPTION_MAP = {
    {KeyboardLifeCycleException::ANIM_SYNC_EXCEPTION, "ANIM_SYNC_EXCEPTION"},
    {KeyboardLifeCycleException::CREATE_EXCEPTION, "CREATE_EXCEPTION"}
};
}
WM_IMPLEMENT_SINGLE_INSTANCE(WindowInfoReporter)

constexpr char EVENT_KEY_BUNDLE_NAME[] = "BUNDLE_NAME";
constexpr char EVENT_KEY_WINDOW_NAME[] = "WINDOW_NAME";
constexpr char EVENT_KEY_MISSION_ID[] = "MISSION_ID";
constexpr char EVENT_KEY_TIMESTAMP[] = "TIMESTAMP";

/**
 * @brief Construct a new Perform Reporter:: Perform Reporter object
 *
 * @param tag A tag that in report string
 * @param timeSpiltsMs The time-interval that data statistic, details look up the comments in function body
 * @param reportInterval Report data after reportInterval round start-end
 */
PerformReporter::PerformReporter(const std::string& tag,
    const std::vector<int64_t>& timeSpiltsMs, uint32_t reportInterval)
    : tag_(tag), reportInterval_(reportInterval)
{
    // re-organ data struct
    // a, b, c, d -->
    // (0, a] : cnt=0, (a, b] : cnt=0, (b, c] : cnt=0, (c, d] : cnt=0
    for (auto split : timeSpiltsMs) {
        timeSplitCount_[split] = 0;
    }
    // (d, +limit] : cnt=0
    timeSplitCount_[BARRIER] = 0;
    totalCount_ = 0;
}

void PerformReporter::start()
{
    startTime_ = std::chrono::steady_clock::now();
}

void PerformReporter::end()
{
    auto currentTime = std::chrono::steady_clock::now();
    int64_t costTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime_).count();

    count(costTime);

    bool repSucc = report();
    if (repSucc) {
        clear();
    }
}

bool PerformReporter::report()
{
    if (totalCount_ < reportInterval_) {
        return false;
    }

    std::ostringstream oss;
    oss << tag_ << ": ";
    auto maxSplit = 0;
    for (const auto& iter: timeSplitCount_) {
        if (iter.first != BARRIER) {
            oss << "BELLOW" << iter.first << "(ms): " << iter.second << ", ";
            maxSplit = iter.first;
        }
    }
    oss << "ABOVE" << maxSplit << "(ms): " << timeSplitCount_[BARRIER];

    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER, tag_,
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, "MSG", oss.str());
    WLOGI("Write HiSysEvent ret:%{public}d", ret);
    return ret == 0;
}

void PerformReporter::count(int64_t costTime)
{
    totalCount_++;
    for (auto& iter: timeSplitCount_) {
        if (costTime <= iter.first) {
            iter.second++;
            break;
        }
    }

    std::ostringstream oss;
    oss << tag_ << " cost " << costTime << "ms, total count " << totalCount_;
    WLOGI("%{public}s", oss.str().c_str());
}

void PerformReporter::clear()
{
    totalCount_ = 0;
    for (auto& iter: timeSplitCount_) {
        iter.second = 0;
    }
}

std::string WindowInfoReporter::GetMsgString(const FullInfoMap& infoMap)
{
    if (infoMap.empty()) {
        return "";
    }
    std::ostringstream oss;
    oss << "{";
    for (auto& bundleInfos : infoMap) {
        if (bundleInfos.second.empty()) {
            continue;
        }
        oss << "{";
        for (auto& packageInfo : bundleInfos.second) {
            oss << "BUNDLE_NAME:" << bundleInfos.first << ",";
            oss << "ABILITY_NAME:" << packageInfo.first << ",";
            oss << "COUNT:" << packageInfo.second;
        }
        oss << "},";
    }
    oss << "};";
    return oss.str();
}

std::string WindowInfoReporter::GetMsgString(const BundleNameMap& infoMap)
{
    if (infoMap.empty()) {
        return "";
    }
    std::ostringstream oss;
    oss << "{";
    for (auto& bundleInfo : infoMap) {
        oss << "{";
        oss << "BUNDLE_NAME:" << bundleInfo.first << ",";
        oss << "COUNT:" << bundleInfo.second;
        oss << "},";
    }
    oss << "};";
    return oss.str();
}

void WindowInfoReporter::InsertCreateReportInfo(const std::string& bundleName)
{
    UpdateReportInfo(windowCreateReportInfos_, bundleName);
}

void WindowInfoReporter::InsertShowReportInfo(const std::string& bundleName)
{
    UpdateReportInfo(windowShowReportInfos_, bundleName);
}

void WindowInfoReporter::InsertHideReportInfo(const std::string& bundleName)
{
    UpdateReportInfo(windowHideReportInfos_, bundleName);
}

void WindowInfoReporter::InsertDestroyReportInfo(const std::string& bundleName)
{
    UpdateReportInfo(windowDestoryReportInfos_, bundleName);
}

void WindowInfoReporter::InsertNavigationBarReportInfo(const std::string& bundleName, const std::string& packageName)
{
    UpdateReportInfo(windowNavigationBarReportInfos_, bundleName, packageName);
}

void WindowInfoReporter::UpdateReportInfo(FullInfoMap& infoMap,
    const std::string& bundleName, const std::string& packageName)
{
    if (bundleName.empty() || packageName.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    auto iter = infoMap.find(bundleName);
    if (iter == infoMap.end()) {
        std::map<std::string, uint32_t> infos;
        infos.insert(std::make_pair(packageName, 1));
        infoMap.insert(std::make_pair(bundleName, infos));
        return;
    }

    auto countPairIter = iter->second.find(packageName);
    if (countPairIter == iter->second.end()) {
        iter->second.insert(std::make_pair(packageName, 1));
        return;
    }
    infoMap[bundleName][packageName]++;
}

void WindowInfoReporter::UpdateReportInfo(BundleNameMap& infoMap, const std::string& bundleName)
{
    if (bundleName.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    auto iter = infoMap.find(bundleName);
    if (iter == infoMap.end()) {
        infoMap.insert(std::make_pair(bundleName, 1));
        return;
    }
    infoMap[bundleName]++;
}

void WindowInfoReporter::ReportBackButtonInfoImmediately()
{
    Report("WM_REPORT_BACK_KEYEVENT", "Click Back Button");
}

void WindowInfoReporter::ReportZeroOpacityInfoImmediately(const std::string& bundleName, const std::string& packageName)
{
    if (bundleName.empty()) {
        return;
    }
    std::ostringstream oss;
    oss << "{ PROCESS_NAME:" << bundleName.c_str() << ", PACKAGE_NAME:" << "" << packageName.c_str() << " }";
    Report("WM_REPORT_WINDOW_OPACITY_ZERO", oss.str());
}

void WindowInfoReporter::ReportStartWindow(const std::string& bundleName, const std::string& windowName)
{
    std::string eventName = "START_WINDOW";
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER, eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        EVENT_KEY_BUNDLE_NAME, bundleName,
        EVENT_KEY_WINDOW_NAME, windowName);
    if (ret != 0) {
        WLOGFE("Write HiSysEvent error, ret:%{public}d", ret);
    }
}

void WindowInfoReporter::ReportRecordedInfos()
{
    std::lock_guard<std::mutex> lock(mtx_);
    WLOGFD("----Report HiSysEvent write all-----");
    Report("WM_REPORT_WINDOW_CREATE", GetMsgString(windowCreateReportInfos_));
    Report("WM_REPORT_WINDOW_SHOW", GetMsgString(windowShowReportInfos_));
    Report("WM_REPORT_WINDOW_HIDE", GetMsgString(windowHideReportInfos_));
    Report("WM_REPORT_WINDOW_DESTORY", GetMsgString(windowDestoryReportInfos_));
    Report("WM_REPORT_HIDE_NAVIGATIONBAR", GetMsgString(windowNavigationBarReportInfos_));
    ClearRecordedInfos();
}

void WindowInfoReporter::ReportContainerStartBegin(int32_t missionId, const std::string& bundleName, int64_t timestamp)
{
    std::string eventName = "CONTAINER_START_BEGIN";
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER, eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        EVENT_KEY_MISSION_ID, missionId,
        EVENT_KEY_BUNDLE_NAME, bundleName,
        EVENT_KEY_TIMESTAMP, timestamp);
    if (ret != 0) {
        WLOGFE("Write HiSysEvent error, ret:%{public}d", ret);
    }
}

void WindowInfoReporter::Report(const std::string& reportTag, const std::string& msg)
{
    if (msg.empty()) {
        return;
    }
    WLOGFD("Report Tag : [%{public}s], Msg: %{public}s", reportTag.c_str(), msg.c_str());
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER, reportTag,
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, "MSG", msg);
    if (ret != 0) {
        WLOGFE("Write HiSysEvent error, ret:%{public}d", ret);
    }
}

void WindowInfoReporter::ClearRecordedInfos()
{
    WLOGFD("Clear all hiSysEvent write information");
    windowCreateReportInfos_.clear();
    windowShowReportInfos_.clear();
    windowHideReportInfos_.clear();
    windowDestoryReportInfos_.clear();
    windowNavigationBarReportInfos_.clear();
}

int32_t WindowInfoReporter::ReportWindowProfileInfo(const WindowProfileSum& windowProfileSum)
{
    std::string eventName = "WINDOW_PROFILE_STATISTIC";
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER, eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "WINDOW_DETAIL", windowProfileSum.windowInfo,
        "TOTAL_WINDOW_COUNT", windowProfileSum.totalWindowCount,
        "VISIBLE_WINDOW_COUNT", windowProfileSum.visibleWindowCount,
        "INVISIBLE_WINDOW_COUNT", windowProfileSum.invisibleWindowCount,
        "MINIMIZE_WINDOW_COUNT", windowProfileSum.minimizeWindowCount);
    if (ret != 0) {
        WLOGFE("Write HiSysEvent error, ret:%{public}d", ret);
    }
    return ret;
}

void WindowInfoReporter::ReportWindowException(int32_t detectionType, int32_t pid, const std::string& windowInfo)
{
    std::string eventName = "WINDOW_EXCEPTION_DETECTION";
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER, eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "DETECTION_TYPE", detectionType,
        "PID", pid,
        "MSG", windowInfo);
    if (ret != 0) {
        WLOGFE("Write HiSysEvent error, ret:%{public}d", ret);
    }
}

int32_t WindowInfoReporter::ReportUIExtensionException(int32_t exceptionType, int32_t pid, int32_t persistentId,
    const std::string& uiextInfo)
{
    std::string eventName = "UIEXTENSION_EXCEPTION";
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER, eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "EXCEPTION_TYPE", exceptionType,
        "PID", pid,
        "PERSISTENTID", persistentId,
        "MSG", uiextInfo);
    if (ret != 0) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write HiSysEvent error, ret: %{public}d", ret);
    }
    return ret;
}

int32_t WindowInfoReporter::ReportEventDispatchException(int32_t exceptionType, int32_t pid,
    const std::string& flushInfo)
{
    std::string eventName = "EVENT_DISPATCH_EXCEPTION";
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER, eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "EXCEPTION_TYPE", exceptionType,
        "PID", pid,
        "MSG", flushInfo);
    if (ret != 0) {
        TLOGE(WmsLogTag::WMS_EVENT, "Write HiSysEvent error, ret: %{public}d", ret);
    }
    return ret;
}

int32_t WindowInfoReporter::ReportKeyboardLifeCycleException(int32_t windowId, KeyboardLifeCycleException subType,
    const std::string& msg)
{
    std::string eventName = "KEYBOARD_LIFE_CYCLE_EXCEPTION";
    int32_t ret = HiSysEventWrite(
        HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER, eventName,
        HiviewDFX::HiSysEvent::EventType::FAULT,
        "WINDOW_ID", windowId,
        "SUB_TYPE", KEYBOARD_LIFE_CYCLE_EXCEPTION_MAP.at(subType),
        "MSG", msg);
    if (ret != 0) {
        TLOGE(WmsLogTag::DEFAULT, "write HiSysEvent error, ret: %{public}d", ret);
    }
    return ret;
}

int32_t WindowInfoReporter::ReportSpecWindowLifeCycleChange(WindowLifeCycleReportInfo reportInfo)
{
    std::string eventName = "SPEC_WINDOW_LIFE_CYCLE_CHANGE";
    int32_t ret = HiSysEventWrite(
        HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER, eventName,
        HiviewDFX::HiSysEvent::EventType::FAULT,
        "BUNDLE_NAME", reportInfo.bundleName,
        "WINDOW_ID", reportInfo.windowId,
        "WINDOW_TYPE", reportInfo.windowType,
        "WINDOW_MODE", reportInfo.windowMode,
        "WINDOW_FLAG", reportInfo.windowFlag,
        "STAGE", reportInfo.timeoutStage);
    if (ret != 0) {
        TLOGE(WmsLogTag::DEFAULT, "write HiSysEvent error, ret: %{public}d", ret);
    }
    return ret;
}
} // namespace Rosen
}