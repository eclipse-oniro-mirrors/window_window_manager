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

#include <gtest/gtest.h>
#include "dms_reporter.h"
#include "hilog/log.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg = msg;
    }
}
namespace OHOS {
namespace Rosen {
class DmsReporterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void DmsReporterTest::SetUpTestCase() {}

void DmsReporterTest::TearDownTestCase() {}

void DmsReporterTest::SetUp() {}

void DmsReporterTest::TearDown() {}

namespace {
/**
 * @tc.name: ReportRegisterSessionListener
 * @tc.desc: ReportRegisterSessionListener test
 * @tc.type: FUNC
 */
HWTEST_F(DmsReporterTest, ReportRegisterSessionListener, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DmsReporter dmsReporter;
    dmsReporter.ReportRegisterSessionListener(true, 0);
    EXPECT_TRUE(g_logMsg.find("Write HiSysEvent error") == std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ReportQuerySessionInfo
 * @tc.desc: ReportQuerySessionInfo test
 * @tc.type: FUNC
 */
HWTEST_F(DmsReporterTest, ReportQuerySessionInfo, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DmsReporter dmsReporter;
    dmsReporter.ReportQuerySessionInfo(true, 0);
    EXPECT_TRUE(g_logMsg.find("Write HiSysEvent error") == std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ReportContinueApp
 * @tc.desc: ReportContinueApp test
 * @tc.type: FUNC
 */
HWTEST_F(DmsReporterTest, ReportContinueApp, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DmsReporter dmsReporter;
    dmsReporter.ReportContinueApp(true, 0);
    EXPECT_TRUE(g_logMsg.find("Write HiSysEvent error") == std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

} // namespace
} // namespace Rosen
} // namespace OHOS