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

#ifndef OHOS_ROSEN_WM_UTILS_PERFORM_REPORTER_H
#define OHOS_ROSEN_WM_UTILS_PERFORM_REPORTER_H

#include <atomic>
#include <chrono>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <sstream>
#include "wm_single_instance.h"

namespace OHOS {
namespace Rosen {

enum class WindowDFXHelperType : uint32_t {
    WINDOW_RECT_CHECK = 1,
    WINDOW_ZORDER_CHECK,
    WINDOW_FOCUS_CHECK,
    WINDOW_TRANSPARENT_CHECK,
    WINDOW_MODAL_UIEXTENSION_UICONTENT_CHECK,
    WINDOW_MODAL_UIEXTENSION_SUBWINDOW_CHECK,
    WINDOW_UIEXTENSION_TRANSFER_DATA_FAIL,
    WINDOW_UIEXTENSION_START_ABILITY_FAIL,
    WINDOW_FLUSH_EMPTY_DISPLAY_INFO_TO_MMI_EXCEPTION,
    WINDOW_CREATE_SUB_WINDOW_FAILED,
};

struct WindowProfileInfo {
    std::string bundleName = "";
    int32_t windowVisibleState = -1;
    int32_t windowLocatedScreen = -1;
    int32_t windowSceneMode = -1;
    std::string rect = "";
    int32_t zorder = -1;
};

enum class KeyboardLifeCycleException {
    ANIM_SYNC_EXCEPTION,
    CREATE_EXCEPTION,
};

static constexpr size_t WINDOW_PROFILE_STATISTIC_SIZE = 50;
struct WindowProfileSum {
    std::vector<std::string> windowInfo = std::vector<std::string>(WINDOW_PROFILE_STATISTIC_SIZE, "");
    int32_t totalWindowCount = -1;
    int32_t visibleWindowCount = -1;
    int32_t invisibleWindowCount = -1;
    int32_t minimizeWindowCount = -1;
};

struct WindowLifeCycleReportInfo {
    std::string bundleName;
    int32_t windowId;
    int32_t windowType;
    int32_t windowMode;
    int32_t windowFlag;
    std::string timeoutStage;

    inline std::string ToString() const
    {
        std::stringstream ss;
        ss << "[bundleName:" << bundleName << ", id:" << windowId << ", type:" << windowType << ", windowMode:" <<
            windowMode << ", flag:" << windowFlag << "]";
        return ss.str();
    }
};

class PerformReporter {
public:
    PerformReporter(const std::string& tag, const std::vector<int64_t>& timeSpiltsMs, uint32_t reportInterval = 50);
    void start();
    void end();

private:
    void count(int64_t costTime);
    bool report();
    void clear();

    std::string tag_;
    std::atomic<uint32_t> totalCount_;
    std::map<int64_t, std::atomic<uint32_t>> timeSplitCount_;
    std::chrono::steady_clock::time_point startTime_;
    uint32_t reportInterval_;

    static constexpr auto BARRIER = std::numeric_limits<int64_t>::max();
};

// the map form : <bundleName, <abilityName, count>>
using FullInfoMap = std::map<std::string, std::map<std::string, uint32_t>>;
// the map form : <bundleName, count>
using BundleNameMap = std::map<std::string, uint32_t>;
class WindowInfoReporter {
WM_DECLARE_SINGLE_INSTANCE(WindowInfoReporter);

public:
    void InsertCreateReportInfo(const std::string& bundleName);
    void InsertShowReportInfo(const std::string& bundleName);
    void InsertHideReportInfo(const std::string& bundleName);
    void InsertDestroyReportInfo(const std::string& bundleName);

    void InsertRecentReportInfo(const std::string& bundleName, const std::string& packageName);
    void InsertNavigationBarReportInfo(const std::string& bundleName, const std::string& packageName);

    void ReportBackButtonInfoImmediately();
    void ReportZeroOpacityInfoImmediately(const std::string& bundleName, const std::string& packageName);
    // report when application startup request window
    void ReportStartWindow(const std::string& bundleName, const std::string& windowName);
    void ReportRecordedInfos();
    void ReportContainerStartBegin(int32_t missionId, const std::string& bundleName, int64_t timestamp);
    int32_t ReportWindowProfileInfo(const WindowProfileSum& windowProfileSum);
    void ReportWindowException(int32_t detectionType, int32_t pid, const std::string& windowInfo);
    int32_t ReportUIExtensionException(int32_t exceptionType, int32_t pid, int32_t persistentId,
        const std::string& uiextInfo);
    int32_t ReportEventDispatchException(int32_t exceptionType, int32_t pid, const std::string& flushInfo);
    int32_t ReportKeyboardLifeCycleException(int32_t windowId, KeyboardLifeCycleException subType,
        const std::string& msg);
    int32_t ReportSpecWindowLifeCycleChange(WindowLifeCycleReportInfo reportInfo);

private:
    void UpdateReportInfo(FullInfoMap& infoMap, const std::string& bundleName,
        const std::string& packageName);
    void UpdateReportInfo(BundleNameMap& infoMap, const std::string& bundleName);
    std::string GetMsgString(const FullInfoMap& infoMap);
    std::string GetMsgString(const BundleNameMap& infoMap);

    void Report(const std::string& reportTag, const std::string& msg);
    void ClearRecordedInfos();

    BundleNameMap windowCreateReportInfos_;
    BundleNameMap windowShowReportInfos_;
    BundleNameMap windowHideReportInfos_;
    BundleNameMap windowDestoryReportInfos_;
    FullInfoMap windowRecentReportInfos_;
    FullInfoMap windowNavigationBarReportInfos_;

    std::mutex mtx_;
};
}
}
#endif