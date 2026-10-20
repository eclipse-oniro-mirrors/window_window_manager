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

#include <fcntl.h>
#include <gtest/gtest.h>

#include "display_manager.h"
#include "window_dumper.h"
#include "window_manager_service.h"
#include "window_impl.h"
#include "window_agent.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowDumperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowDumperTest::SetUpTestCase()
{
    auto display = DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE((display != nullptr));
    sptr<DisplayInfo> displayInfo = display->GetDisplayInfo();
    ASSERT_TRUE((displayInfo != nullptr));

    displayInfo->SetScreenGroupId(0);
    WindowManagerService::GetInstance().windowRoot_->CreateWindowNodeContainer(0, displayInfo);
}

void WindowDumperTest::TearDownTestCase()
{
}

void WindowDumperTest::SetUp()
{
}

void WindowDumperTest::TearDown()
{
}

namespace {
/**
 * @tc.name: Dump02
 * @tc.desc: Dump fd less 0
 * @tc.type: FUNC
 */
HWTEST_F(WindowDumperTest, Dump02, TestSize.Level1)
{
    sptr<WindowDumper> windowDumper;
    windowDumper = new WindowDumper(WindowManagerService::GetInstance().windowRoot_);
    int fd = -1;
    std::vector<std::u16string> args;
    WMError ret = windowDumper->Dump(fd, args);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: Dump04
 * @tc.desc: Dump one param with '-x'
 * @tc.type: FUNC
 */
HWTEST_F(WindowDumperTest, Dump04, TestSize.Level1)
{
    sptr<WindowDumper> windowDumper;
    windowDumper = new WindowDumper(WindowManagerService::GetInstance().windowRoot_);
    int fd = 2;
    std::vector<std::u16string> args;
    const std::u16string DUMP_HELP = u"-x";
    args.emplace_back(DUMP_HELP);
    WMError ret = windowDumper->Dump(fd, args);
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: Dump05
 * @tc.desc: Dump two param with '-a'
 * @tc.type: FUNC
 */
HWTEST_F(WindowDumperTest, Dump05, TestSize.Level1)
{
    sptr<WindowDumper> windowDumper;
    windowDumper = new WindowDumper(WindowManagerService::GetInstance().windowRoot_);
    int fd = 3;
    std::vector<std::u16string> args;
    const std::u16string DUMP_ALL = u"-a";
    args.emplace_back(DUMP_ALL);
    WMError ret = windowDumper->Dump(fd, args);
    ASSERT_TRUE(ret == WMError::WM_OK || ret == WMError::WM_ERROR_INVALID_OPERATION);
}

/**
 * @tc.name: Dump06
 * @tc.desc: Dump two param with '-w 1'
 * @tc.type: FUNC
 */
HWTEST_F(WindowDumperTest, Dump06, TestSize.Level1)
{
    sptr<WindowDumper> windowDumper;
    windowDumper = new WindowDumper(WindowManagerService::GetInstance().windowRoot_);
    int fd = 4;
    std::vector<std::u16string> args;
    const std::u16string DUMP_WINDOW = u"-w";
    const std::u16string DUMP_WINDOW_ID = u"3";
    args.emplace_back(DUMP_WINDOW);
    args.emplace_back(DUMP_WINDOW_ID);
    WMError ret = windowDumper->Dump(fd, args);
    ASSERT_TRUE(ret == WMError::WM_OK || ret == WMError::WM_ERROR_INVALID_OPERATION);
}

/**
 * @tc.name: Dump07
 * @tc.desc: Dump two param with '-w n'
 * @tc.type: FUNC
 */
HWTEST_F(WindowDumperTest, Dump07, TestSize.Level1)
{
    sptr<WindowDumper> windowDumper;
    windowDumper = new WindowDumper(WindowManagerService::GetInstance().windowRoot_);
    int fd = 5;
    std::vector<std::u16string> args;
    const std::u16string DUMP_WINDOW = u"-w";
    const std::u16string DUMP_WINDOW_ID = u"n";
    args.emplace_back(DUMP_WINDOW);
    args.emplace_back(DUMP_WINDOW_ID);
    WMError ret = windowDumper->Dump(fd, args);
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: Dump08
 * @tc.desc: Dump with out of range window id, should be rejected without crash
 * @tc.type: FUNC
 */
HWTEST_F(WindowDumperTest, Dump08, TestSize.Level1)
{
    sptr<WindowDumper> windowDumper;
    windowDumper = new WindowDumper(WindowManagerService::GetInstance().windowRoot_);
    int fd = 2;
    std::vector<std::u16string> args;
    const std::u16string DUMP_WINDOW = u"-w";
    const std::u16string DUMP_WINDOW_ID_OUT_OF_RANGE = u"99999999999"; // exceeds ULONG_MAX on 32-bit
    const std::u16string DUMP_WINDOW_ID_OVERFLOW_U32 = u"4294967296"; // 2^32, exceeds UINT32_MAX
    args.emplace_back(DUMP_WINDOW);
    args.emplace_back(DUMP_WINDOW_ID_OUT_OF_RANGE);
    WMError ret = windowDumper->Dump(fd, args);
    ASSERT_EQ(ret, WMError::WM_OK);
    args[1] = DUMP_WINDOW_ID_OVERFLOW_U32;
    ret = windowDumper->Dump(fd, args);
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: ShowAceDumpHelp01
 * @tc.desc: ShowAceDumpHelp
 * @tc.type: FUNC
 */
HWTEST_F(WindowDumperTest, ShowAceDumpHelp01, TestSize.Level1)
{
    sptr<WindowDumper> windowDumper;
    windowDumper = new WindowDumper(WindowManagerService::GetInstance().windowRoot_);
    std::string dumpInfo;
    windowDumper->ShowAceDumpHelp(dumpInfo);
    WindowManagerService::GetInstance().windowRoot_->windowNodeMap_.clear();
    ASSERT_TRUE(dumpInfo.empty());
}

/**
 * @tc.name: ShowAceDumpHelp02
 * @tc.desc: ShowAceDumpHelp
 * @tc.type: FUNC
 */
HWTEST_F(WindowDumperTest, ShowAceDumpHelp02, TestSize.Level1)
{
    sptr<WindowDumper> windowDumper;
    windowDumper = new WindowDumper(WindowManagerService::GetInstance().windowRoot_);
    std::string dumpInfo;
    windowDumper->ShowAceDumpHelp(dumpInfo);
    WindowManagerService::GetInstance().windowRoot_->windowNodeMap_.clear();
    ASSERT_TRUE(dumpInfo.empty());
}

/**
 * @tc.name: CompleteDumpInfoRequest01
 * @tc.desc: Complete dump info request with caller identity not matching pending request
 * @tc.type: FUNC
 */
HWTEST_F(WindowDumperTest, CompleteDumpInfoRequest01, TestSize.Level1)
{
    sptr<WindowDumper> windowDumper;
    windowDumper = new WindowDumper(WindowManagerService::GetInstance().windowRoot_);
    std::vector<std::string> info = { "test dump info" };
    // no pending dump request
    windowDumper->CompleteDumpInfoRequest(1, 1, info);
    // caller identity does not match pending request
    windowDumper->BeginDumpInfoRequest(1, 1);
    windowDumper->CompleteDumpInfoRequest(2, 1, info);
    windowDumper->CompleteDumpInfoRequest(1, 2, info);
    auto result = windowDumper->dumpInfoFuture_.GetResult(0);
    ASSERT_TRUE(result.empty());
    // caller identity matches pending request
    windowDumper->CompleteDumpInfoRequest(1, 1, info);
    result = windowDumper->dumpInfoFuture_.GetResult(0);
    ASSERT_EQ(result.size(), 1UL);
    ASSERT_EQ(result[0], "test dump info");
    windowDumper->CancelDumpInfoRequest();
}
}
}
}