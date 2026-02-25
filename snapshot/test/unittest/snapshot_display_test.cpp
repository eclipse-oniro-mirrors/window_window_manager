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

#include <cinttypes>
#include <cstdlib>
#include <gtest/gtest.h>

#include "pixel_map.h"

#include "snapshot_utils.h"
#include "common_test_utils.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "SnapshotDisplayTest"};
}

class SnapshotDisplayTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    static DisplayId defaultId_;
    DisplayId invalidId_ = DISPLAY_ID_INVALID;
    const std::string defaultCmd_ = "/system/bin/snapshot_display";
    const int testTimeCount_ = 2;
};

DisplayId SnapshotDisplayTest::defaultId_ = DISPLAY_ID_INVALID;

void SnapshotDisplayTest::SetUpTestCase()
{
    auto display = DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        WLOGFE("GetDefaultDisplay: failed!\n");
        return;
    }
    defaultId_ = display->GetId();
    WLOGI("GetDefaultDisplay: id[%{public}" PRIu64"], w:[%{public}d], h[%{public}d], fps[%{public}u]",
        defaultId_, display->GetWidth(), display->GetHeight(), display->GetRefreshRate());

    CommonTestUtils::InjectTokenInfoByHapName(0, "com.ohos.systemui", 0);
    const char** perms = new const char *[1];
    perms[0] = "ohos.permission.CAPTURE_SCREEN";
    CommonTestUtils::SetAceessTokenPermission("DisplayManagerServiceTest", perms, 1);
}

void SnapshotDisplayTest::TearDownTestCase()
{
}

void SnapshotDisplayTest::SetUp()
{
}

void SnapshotDisplayTest::TearDown()
{
}

bool CheckFileExist(const std::string& fPath)
{
    if (!fPath.empty()) {
        FILE* fp = fopen(fPath.c_str(), "r");
        if (fp != nullptr) {
            fclose(fp);
            return true;
        }
    }
    return false;
}

bool TakeScreenshotBySpecifiedParam(std::string exec, std::string imgPath, std::string extraParam)
{
    if (CheckFileExist(imgPath)) {
        remove(imgPath.c_str());
    }
    const std::string cmd = exec + " -f " + imgPath +  " " + extraParam;
    (void)system(cmd.c_str());
    bool isExist = CheckFileExist(imgPath);
    if (isExist) {
        remove(imgPath.c_str());
    }
    return isExist;
}

namespace {
/**
 * @tc.name: ScreenShotCmdValid
 * @tc.desc: Call screenshot default cmd and check if it saves image in default path
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotDisplayTest, ScreenShotCmdValid01, TestSize.Level1)
{
    std::string imgPath[testTimeCount_];
    int i;

    for (i = 0; i < testTimeCount_; i++) {
        imgPath[i] = SnapShotUtils::GenerateFileName("jpeg", i);
        if (CheckFileExist(imgPath[i])) {
            remove(imgPath[i].c_str());
        }
    }

    const std::string cmd = defaultCmd_ + " -i " + std::to_string(defaultId_) + " -f " + imgPath[0];
    (void)system(cmd.c_str());
    bool isSuccess = false;
    for (i = 0; i < testTimeCount_; i++) {
        isSuccess = CheckFileExist(imgPath[i]);
        if (isSuccess) {  // ok
            remove(imgPath[i].c_str());
            ASSERT_TRUE(true);
            break;
        }
    }
    ASSERT_TRUE(isSuccess); // fail, can't find snapshot file
}

/**
 * @tc.name: ScreenShotCmdValid
 * @tc.desc: Call screenshot with default displayID and default path
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotDisplayTest, ScreenShotCmdValid02, TestSize.Level1)
{
    std::string imgPath[testTimeCount_];
    int i;

    for (i = 0; i < testTimeCount_; i++) {
        imgPath[i] = SnapShotUtils::GenerateFileName("jpeg", i);
        if (CheckFileExist(imgPath[i])) {
            remove(imgPath[i].c_str());
        }
    }

    const std::string cmd = defaultCmd_ + " -i " + std::to_string(defaultId_) + " -f " + imgPath[0];
    (void)system(cmd.c_str());
    bool isSuccess = false;
    for (i = 0; i < testTimeCount_; i++) {
        isSuccess = CheckFileExist(imgPath[i]);
        if (isSuccess) {  // ok
            remove(imgPath[i].c_str());
            ASSERT_TRUE(true);
            break;
        }
    }
    ASSERT_TRUE(isSuccess);  // fail, can't find snapshot file
}

/**
 * @tc.name: ScreenShotCmdValid
 * @tc.desc: Call screenshot with default displayID and custom path
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotDisplayTest, ScreenShotCmdValid03, TestSize.Level1)
{
    const std::string imgPath = "/data/local/tmp/snapshot_display_test.jpeg";
    std::string extraParam = "-i " + std::to_string(defaultId_);
    ASSERT_EQ(true, TakeScreenshotBySpecifiedParam(defaultCmd_, imgPath, extraParam));
}

/**
 * @tc.name: ScreenShotCmdValid
 * @tc.desc: Call screenshot with valid width/height
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotDisplayTest, ScreenShotCmdValid04, TestSize.Level1)
{
    const std::string imgPath = "/data/local/tmp/snapshot_display_test.jpeg";
    std::string extraParam = "-i " + std::to_string(defaultId_) + " -w 100 -h 100";
    ASSERT_EQ(true, TakeScreenshotBySpecifiedParam(defaultCmd_, imgPath, extraParam));
}

/**
 * @tc.name: ScreenShotCmdValid
 * @tc.desc: Call screenshot with valid width
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotDisplayTest, ScreenShotCmdValid05, TestSize.Level1)
{
    const std::string imgPath = "/data/local/tmp/snapshot_display_test.jpeg";
    std::string extraParam = "-i " + std::to_string(defaultId_) + " -w 100";
    ASSERT_EQ(true, TakeScreenshotBySpecifiedParam(defaultCmd_, imgPath, extraParam));
}

/**
 * @tc.name: ScreenShotCmdValid
 * @tc.desc: Call screenshot with valid height
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotDisplayTest, ScreenShotCmdValid06, TestSize.Level1)
{
    const std::string imgPath = "/data/local/tmp/snapshot_display_test.jpeg";
    std::string extraParam = "-i " + std::to_string(defaultId_) + " -h 100";
    ASSERT_EQ(true, TakeScreenshotBySpecifiedParam(defaultCmd_, imgPath, extraParam));
}

/**
 * @tc.name: ScreenShotCmdValid
 * @tc.desc: Call screenshot with invalid width/height
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotDisplayTest, ScreenShotCmdValid07, TestSize.Level1)
{
    const std::string imgPath = "/data/local/tmp/snapshot_display_test.jpeg";
    std::string extraParam = "-i " + std::to_string(defaultId_) + " -w 10000 -h 10000";
    ASSERT_EQ(false, TakeScreenshotBySpecifiedParam(defaultCmd_, imgPath, extraParam));
}

/**
 * @tc.name: ScreenShotCmdValid
 * @tc.desc: Call screenshot with -m
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotDisplayTest, ScreenShotCmdValid08, TestSize.Level1)
{
    const std::string imgPath = "/data/local/tmp/snapshot_display_test.jpeg";
    std::string extraParam = "-i " + std::to_string(defaultId_) + " -m";
    ASSERT_EQ(false, TakeScreenshotBySpecifiedParam(defaultCmd_, imgPath, extraParam));
}

/**
 * @tc.name: ScreenShotCmdValid
 * @tc.desc: screenshot png type
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotDisplayTest, ScreenShotCmdValid09, TestSize.Level1)
{
    std::string imgPath[testTimeCount_];
    int i;

    for (i = 0; i < testTimeCount_; i++) {
        imgPath[i] = SnapShotUtils::GenerateFileName("png", i);
        if (CheckFileExist(imgPath[i])) {
            remove(imgPath[i].c_str());
        }
    }

    const std::string cmd = defaultCmd_  + " -f " + imgPath[0] + " -t png";
    (void)system(cmd.c_str());
    bool isSuccess = false;
    for (i = 0; i < testTimeCount_; i++) {
        isSuccess = CheckFileExist(imgPath[i]);
        if (isSuccess) {  // ok
            remove(imgPath[i].c_str());
            ASSERT_TRUE(true);
            break;
        }
    }
    ASSERT_TRUE(isSuccess);  // fail, can't find snapshot file
}
} // namespace
} // namespace Rosen
} // namespace OHOS