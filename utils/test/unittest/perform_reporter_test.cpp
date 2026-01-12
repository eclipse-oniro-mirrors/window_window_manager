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

#include <gtest/gtest.h>
#include <thread>

#include "input_manager.h"
#include "perform_reporter.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "PerformReporterTest" };
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
}
class PerformReporterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    void SimuReportProcess(PerformReporter& pr, const std::vector<uint32_t>& durations);
    bool PerformDataCmp(const PerformReporter& pr, const uint32_t totalCount, const std::vector<uint32_t>& splitCount);
};

void PerformReporterTest::SetUpTestCase() {}

void PerformReporterTest::TearDownTestCase() {}

void PerformReporterTest::SetUp() {}

void PerformReporterTest::TearDown() {}

void PerformReporterTest::SimuReportProcess(PerformReporter& pr, const std::vector<uint32_t>& durations)
{
    for (auto duration : durations) {
        pr.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
        pr.end();
    }
}

bool PerformReporterTest::PerformDataCmp(const PerformReporter& pr,
                                         const uint32_t totalCount,
                                         const std::vector<uint32_t>& splitCount)
{
    if (pr.totalCount_ != totalCount) {
        WLOGFE("pr.totalCount_=%{public}u, expect=%{public}u", pr.totalCount_.load(), totalCount);
        return false;
    }

    size_t i = 0;
    for (auto& iter : pr.timeSplitCount_) {
        if (iter.second != splitCount[i]) {
            std::ostringstream oss;
            oss << "pr.timeSplitCount_[" << iter.first << "]=" << iter.second << ", but expect=" << splitCount[i];
            WLOGI("%{public}s", oss.str().c_str());
            return false;
        }
        i++;
    }

    return true;
}

namespace {
/**
 * @tc.name: StartEnd
 * @tc.desc: StartEnd test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, StartEnd, TestSize.Level1)
{
    PerformReporter pr = PerformReporter("TestTag", { 100, 200, 300 }, 10);
    SimuReportProcess(pr, { 50, 150, 250, 350, 450 });
    ASSERT_EQ(true, PerformDataCmp(pr, 5, { 1, 1, 1, 2 }));
}

/**
 * @tc.name: StartEndClear
 * @tc.desc: StartEndClear test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, StartEndClear, TestSize.Level1)
{
    PerformReporter pr = PerformReporter("TestTag", { 100, 200, 300 }, 3);
    SimuReportProcess(pr, { 50, 150, 250 });
    ASSERT_EQ(true, PerformDataCmp(pr, 0, { 0, 0, 0, 0 }));
}

/**
 * @tc.name: StartEndInvSeq
 * @tc.desc: StartEndInvSeq test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, StartEndInvSeq, TestSize.Level1)
{
    PerformReporter pr = PerformReporter("TestTag", { 100, 200, 300 }, 4);
    SimuReportProcess(pr, { 250, 150, 50 });
    ASSERT_EQ(true, PerformDataCmp(pr, 3, { 1, 1, 1, 0 }));
}

/**
 * @tc.name: PrivateClear
 * @tc.desc: PrivateClear test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, PrivateClear, TestSize.Level1)
{
    PerformReporter pr = PerformReporter("TestTag", { 100, 200, 300 }, 10);
    SimuReportProcess(pr, { 50, 150, 250, 350, 450 });
    ASSERT_EQ(true, PerformDataCmp(pr, 5, { 1, 1, 1, 2 }));

    pr.clear();
    ASSERT_EQ(true, PerformDataCmp(pr, 0, { 0, 0, 0, 0 }));
}

/**
 * @tc.name: GetMsgString001
 * @tc.desc: GetMsgString test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, GetMsgString001, TestSize.Level1)
{
    WindowInfoReporter windowInfoReporter;
    FullInfoMap infoMap;
    std::string res = windowInfoReporter.GetMsgString(infoMap);
    ASSERT_EQ(res, "");
}

/**
 * @tc.name: GetMsgString002
 * @tc.desc: GetMsgString test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, GetMsgString002, TestSize.Level1)
{
    WindowInfoReporter windowInfoReporter;
    FullInfoMap infoMap;
    infoMap["bundleName"]["packageName"] = 0;
    std::string res = windowInfoReporter.GetMsgString(infoMap);
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
    ASSERT_EQ(res, oss.str());
}

/**
 * @tc.name: GetMsgString003
 * @tc.desc: GetMsgString test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, GetMsgString003, TestSize.Level1)
{
    WindowInfoReporter windowInfoReporter;
    BundleNameMap infoMap;
    std::string res = windowInfoReporter.GetMsgString(infoMap);
    ASSERT_EQ(res, "");
}

/**
 * @tc.name: GetMsgString004
 * @tc.desc: GetMsgString test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, GetMsgString004, TestSize.Level1)
{
    WindowInfoReporter windowInfoReporter;
    BundleNameMap infoMap;
    infoMap["bundleName"] = 0;
    std::string res = windowInfoReporter.GetMsgString(infoMap);
    std::ostringstream oss;
    oss << "{";
    for (auto& bundleInfo : infoMap) {
        oss << "{";
        oss << "BUNDLE_NAME:" << bundleInfo.first << ",";
        oss << "COUNT:" << bundleInfo.second;
        oss << "},";
    }
    oss << "};";
    ASSERT_EQ(res, oss.str());
}

/**
 * @tc.name: InsertCreateReportInfo005
 * @tc.desc: InsertCreateReportInfo test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, InsertCreateReportInfo005, TestSize.Level1)
{
    WindowInfoReporter windowInfoReporter;
    std::string bundleName = "bundleName";
    std::string packageName = "packageName";
    windowInfoReporter.InsertCreateReportInfo(bundleName);
    windowInfoReporter.InsertShowReportInfo(bundleName);
    windowInfoReporter.InsertHideReportInfo(bundleName);
    windowInfoReporter.InsertDestroyReportInfo(bundleName);
    windowInfoReporter.InsertNavigationBarReportInfo(bundleName, packageName);
    EXPECT_NE(0, windowInfoReporter.windowCreateReportInfos_.size());
    EXPECT_NE(0, windowInfoReporter.windowShowReportInfos_.size());
    EXPECT_NE(0, windowInfoReporter.windowHideReportInfos_.size());
}

/**
 * @tc.name: UpdateReportInfo006
 * @tc.desc: UpdateReportInfo test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, UpdateReportInfo006, TestSize.Level1)
{
    WindowInfoReporter windowInfoReporter;
    FullInfoMap infoMap;
    std::string bundleName;
    std::string packageName;
    windowInfoReporter.UpdateReportInfo(infoMap, bundleName, packageName);
    std::string res = windowInfoReporter.GetMsgString(infoMap);
    ASSERT_EQ(res, "");
}

/**
 * @tc.name: UpdateReportInfo007
 * @tc.desc: UpdateReportInfo test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, UpdateReportInfo007, TestSize.Level1)
{
    WindowInfoReporter windowInfoReporter;
    FullInfoMap infoMap_1;
    std::string bundleName = "bundleName";
    std::string packageName = "packageName";
    infoMap_1["bundleName"]["packageName"] = 0;
    windowInfoReporter.UpdateReportInfo(infoMap_1, bundleName, packageName);
    EXPECT_EQ(1, infoMap_1["bundleName"]["packageName"]);
    FullInfoMap infoMap_2;
    infoMap_2["Name"]["packageName"] = 0;
    windowInfoReporter.UpdateReportInfo(infoMap_2, bundleName, packageName);
    EXPECT_EQ(1, infoMap_2["bundleName"]["packageName"]);
}

/**
 * @tc.name: UpdateReportInfo008
 * @tc.desc: UpdateReportInfo test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, UpdateReportInfo008, TestSize.Level1)
{
    WindowInfoReporter windowInfoReporter;
    BundleNameMap infoMap;
    std::string bundleName;
    windowInfoReporter.UpdateReportInfo(infoMap, bundleName);
    std::string res = windowInfoReporter.GetMsgString(infoMap);
    ASSERT_EQ(res, "");
}

/**
 * @tc.name: UpdateReportInfo009
 * @tc.desc: UpdateReportInfo test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, UpdateReportInfo009, TestSize.Level1)
{
    WindowInfoReporter windowInfoReporter;
    BundleNameMap infoMap_1;
    std::string bundleName = "bundleName";
    infoMap_1["bundleName"] = 0;
    windowInfoReporter.UpdateReportInfo(infoMap_1, bundleName);
    BundleNameMap infoMap_2;
    infoMap_2["Name"] = 0;
    windowInfoReporter.UpdateReportInfo(infoMap_2, bundleName);
    EXPECT_EQ(1, infoMap_2["bundleName"]);
}

/**
 * @tc.name: ReportBackButtonInfoImmediately010
 * @tc.desc: ReportBackButtonInfoImmediately test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, ReportBackButtonInfoImmediately010, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    WindowInfoReporter windowInfoReporter;
    windowInfoReporter.ReportBackButtonInfoImmediately();
    EXPECT_FALSE(g_errLog.find("WM_REPORT_BACK_KEYEVENT") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ReportZeroOpacityInfoImmediately011
 * @tc.desc: ReportZeroOpacityInfoImmediately test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, ReportZeroOpacityInfoImmediately011, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    std::string bundleName;
    std::string packageName = "packageName";
    WindowInfoReporter windowInfoReporter;
    windowInfoReporter.ReportZeroOpacityInfoImmediately(bundleName, packageName);
    bundleName = "bundleName";
    windowInfoReporter.ReportZeroOpacityInfoImmediately(bundleName, packageName);
    EXPECT_FALSE(g_errLog.find("WM_REPORT_WINDOW_OPACITY_ZERO") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ReportStartWindow012
 * @tc.desc: ReportStartWindow test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, ReportStartWindow012, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    std::string bundleName = "bundleName";
    std::string windowName = "windowName";
    WindowInfoReporter windowInfoReporter;
    windowInfoReporter.ReportStartWindow(bundleName, windowName);
    EXPECT_FALSE(g_errLog.find("Write HiSysEvent error,") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ReportRecordedInfos013
 * @tc.desc: ReportRecordedInfos test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, ReportRecordedInfos013, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    WindowInfoReporter windowInfoReporter;
    windowInfoReporter.ReportRecordedInfos();
    EXPECT_FALSE(g_errLog.find("----Report HiSysEvent write all-----") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ReportContainerStartBegin014
 * @tc.desc: ReportContainerStartBegin test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, ReportContainerStartBegin014, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    int32_t missionId = 1;
    std::string bundleName = "bundleName";
    int64_t timestamp = 1;
    WindowInfoReporter windowInfoReporter;
    windowInfoReporter.ReportContainerStartBegin(missionId, bundleName, timestamp);
    EXPECT_FALSE(g_errLog.find("Write HiSysEvent error,") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: Report015
 * @tc.desc: Report test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, Report015, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    std::string reportTag = "reportTag";
    std::string msg;
    WindowInfoReporter windowInfoReporter;
    windowInfoReporter.Report(reportTag, msg);
    EXPECT_FALSE(g_errLog.find("Report Tag :") != std::string::npos);
    msg = "msg";
    g_errLog.clear();
    windowInfoReporter.Report(reportTag, msg);
    EXPECT_FALSE(g_errLog.find("Write HiSysEvent error,") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ClearRecordedInfos016
 * @tc.desc: ClearRecordedInfos test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, ClearRecordedInfos016, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    WindowInfoReporter windowInfoReporter;
    windowInfoReporter.ClearRecordedInfos();
    EXPECT_FALSE(g_errLog.find("Clear all hiSysEvent write information") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ReportUIExtensionException
 * @tc.desc: ReportUIExtensionException test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, ReportUIExtensionException, TestSize.Level1)
{
    int32_t res = 0;
    WindowInfoReporter windowInfoReporter;
    WindowDFXHelperType exceptionType = WindowDFXHelperType::WINDOW_UIEXTENSION_TRANSFER_DATA_FAIL;
    int32_t pid = 1111;
    int32_t persistentId = 1111111111;
    // ERR_TRANSACTION_FAILED = 1
    int32_t errorCode = 1;
    std::ostringstream oss;
    oss << "TransferExtensionData from provider to host failed" << ",";
    oss << " provider bundleName: " << "testProviderBundleName1" << ",";
    oss << " provider windowName: " << "testWindowName1" << ",";
    oss << " errorCode: " << errorCode << ";";
    res = windowInfoReporter.ReportUIExtensionException(
        static_cast<int32_t>(exceptionType), pid, persistentId, oss.str());
    ASSERT_EQ(res, 0);

    exceptionType = WindowDFXHelperType::WINDOW_UIEXTENSION_START_ABILITY_FAIL;
    pid = 2222;
    persistentId = 1111122222;
    // ERR_BASE = (-99)
    errorCode = -99;
    oss.str("");
    oss << "Start UIExtensionAbility failed" << ",";
    oss << " provider windowName: " << "testWindowName2" << ",";
    oss << " errorCode: " << errorCode << ";";
    res = windowInfoReporter.ReportUIExtensionException(
        static_cast<int32_t>(exceptionType), pid, persistentId, oss.str());
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: ReportEventDispatchException
 * @tc.desc: ReportEventDispatchException test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, ReportEventDispatchException, TestSize.Level1)
{
    int32_t res = 0;
    WindowInfoReporter windowInfoReporter;
    std::vector<MMI::DisplayInfo> displayInfos;
    ASSERT_EQ(displayInfos.empty(), true);
    WindowDFXHelperType exceptionType = WindowDFXHelperType::WINDOW_FLUSH_EMPTY_DISPLAY_INFO_TO_MMI_EXCEPTION;
    int32_t pid = 1111;
    std::ostringstream oss;
    oss << "displayInfos flush to MMI is empty!";
    res = windowInfoReporter.ReportEventDispatchException(static_cast<int32_t>(exceptionType), pid, oss.str());
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: ReportWindowProfileInfo017
 * @tc.desc: ReportWindowProfileInfo test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, ReportWindowProfileInfo017, TestSize.Level1)
{
    int32_t res = 0;
    WindowProfileSum windowProfileSum;
    windowProfileSum.windowInfo[0] = "bundleName, 3, 0, 102, 115, [121 373 1047 1117];";
    windowProfileSum.totalWindowCount = 1;
    windowProfileSum.visibleWindowCount = 0;
    windowProfileSum.invisibleWindowCount = 1;
    windowProfileSum.minimizeWindowCount = 0;
 
    WindowInfoReporter windowInfoReporter;
    res = windowInfoReporter.ReportWindowProfileInfo(windowProfileSum);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: ReportWindowProfileInfo018
 * @tc.desc: ReportKeyboardLifeCycleException test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, ReportKeyboardLifeCycleException18, Function | SmallTest | Level2)
{
    KeyboardLifeCycleException subEventType = KeyboardLifeCycleException::ANIM_SYNC_EXCEPTION;
    int32_t windowId = 198;
    std::string msg = "ReportKeyboardLifeCycleExceptionTestMSG";
    WindowInfoReporter windowInfoReporter;
    int32_t res = windowInfoReporter.ReportKeyboardLifeCycleException(windowId, subEventType, msg);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: ReportWindowProfileInfo019
 * @tc.desc: ReportKeyboardLifeCycleException test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, ReportKeyboardLifeCycleException19, Function | SmallTest | Level2)
{
    KeyboardLifeCycleException subEventType = KeyboardLifeCycleException::CREATE_EXCEPTION;
    int32_t windowId = 198;
    std::string msg = "ReportKeyboardLifeCycleExceptionTestMSG";
    WindowInfoReporter windowInfoReporter;
    int32_t res = windowInfoReporter.ReportKeyboardLifeCycleException(windowId, subEventType, msg);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: ReportWindowProfileInfo018
 * @tc.desc: ReportSpecWindowLifeCycleChange test
 * @tc.type: FUNC
 */
HWTEST_F(PerformReporterTest, ReportSpecWindowLifeCycleChange, Function | SmallTest | Level2)
{
    int32_t windowId = 198;
    std::string stage = "attach";
    WindowLifeCycleReportInfo reportInfo = { "bundleName", windowId,
        static_cast<int32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW),
        static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN),
        static_cast<int32_t>(WindowFlag::WINDOW_FLAG_IS_TEXT_MENU),
        stage};
    WindowInfoReporter windowInfoReporter;
    int32_t res = windowInfoReporter.ReportSpecWindowLifeCycleChange(reportInfo);
    ASSERT_EQ(res, 0);
}
} // namespace
} // namespace Rosen
} // namespace OHOS