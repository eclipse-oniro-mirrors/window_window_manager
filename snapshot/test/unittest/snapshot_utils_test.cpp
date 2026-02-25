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
#include "display.h"
#include "display_manager.h"
#include "snapshot_utils.h"
#include "common_test_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr int RGB565_PIXEL_BYTES = 2;
constexpr int RGB888_PIXEL_BYTES = 3;
constexpr int BPP = 4;
constexpr int RGBA8888BUF_SIZE = 10;
constexpr int RGB888BUF_SIZE = 10;
constexpr int RGB565BUF_SIZE = 10;
}
class SnapshotUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    const std::string defaultFile_ = "/data/local/tmp/snapshot_display_1.jpeg";
    const int defaultBitDepth_ = 8;
};

void SnapshotUtilsTest::SetUpTestCase()
{
    CommonTestUtils::InjectTokenInfoByHapName(0, "com.ohos.systemui", 0);
    const char** perms = new const char *[1];
    perms[0] = "ohos.permission.CAPTURE_SCREEN";
    CommonTestUtils::SetAceessTokenPermission("DisplayManagerServiceTest", perms, 1);
}

void SnapshotUtilsTest::TearDownTestCase()
{
}

void SnapshotUtilsTest::SetUp()
{
}

void SnapshotUtilsTest::TearDown()
{
}

namespace {
/**
 * @tc.name: Check01
 * @tc.desc: Check if default jpeg is valid file names
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, Check01, TestSize.Level1)
{
    ASSERT_EQ(true, SnapShotUtils::CheckFileNameValid(defaultFile_));
}

/**
 * @tc.name: Check02
 * @tc.desc: Check custom jpeg is valid file names
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, Check02, TestSize.Level1)
{
    std::string fileName = "/data/local/tmp/test.jpeg";
    ASSERT_EQ(true, SnapShotUtils::CheckFileNameValid(fileName));
}

/**
 * @tc.name: Check03
 * @tc.desc: Check random path is invalid file names
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, Check03, TestSize.Level1)
{
    std::string fileName1 = "/path/to/test/1.jpeg";
    ASSERT_EQ(false, SnapShotUtils::CheckFileNameValid(fileName1));
    std::string fileName2 = "";
    ASSERT_EQ(false, SnapShotUtils::CheckFileNameValid(fileName2));
    std::string fileName3 = "/data/test.png";
    ASSERT_EQ(false, SnapShotUtils::CheckFileNameValid(fileName3));
    std::string fileName4 = "test.png";
    ASSERT_EQ(false, SnapShotUtils::CheckFileNameValid(fileName4));
    std::string fileName5 = "/data";
    ASSERT_EQ(false, SnapShotUtils::CheckFileNameValid(fileName5));
    std::string fileName6 = "/data/local/tmp/test.png";
    ASSERT_EQ(false, SnapShotUtils::CheckFileNameValid(fileName6));
}

/**
 * @tc.name: RGBA8888ToRGB88801
 * @tc.desc: RGBA8888 to RGB888 using invalid params
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, RGBA8888ToRGB88801, TestSize.Level1)
{
    uint8_t rgba8888Buf[RGBA8888BUF_SIZE];
    uint8_t rgb888Buf[RGB888BUF_SIZE];
    EXPECT_FALSE(SnapShotUtils::RGBA8888ToRGB888(rgba8888Buf, nullptr, RGBA8888BUF_SIZE));
    EXPECT_FALSE(SnapShotUtils::RGBA8888ToRGB888(nullptr, rgb888Buf, RGB888BUF_SIZE));
    EXPECT_FALSE(SnapShotUtils::RGBA8888ToRGB888(rgba8888Buf, rgb888Buf, 0));
    EXPECT_TRUE(SnapShotUtils::RGBA8888ToRGB888(rgba8888Buf, rgb888Buf, RGBA8888BUF_SIZE));
}

/**
 * @tc.name: RGB565ToRGB888
 * @tc.desc: RGB565 to RGB888 using invalid params
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, RGB565ToRGB888, TestSize.Level1)
{
    uint8_t rgb565Buf[RGB565BUF_SIZE];
    uint8_t rgb888Buf[RGB888BUF_SIZE];
    EXPECT_FALSE(SnapShotUtils::RGB565ToRGB888(rgb565Buf, nullptr, RGB565BUF_SIZE));
    EXPECT_FALSE(SnapShotUtils::RGB565ToRGB888(nullptr, rgb888Buf, RGB888BUF_SIZE));
    EXPECT_FALSE(SnapShotUtils::RGB565ToRGB888(rgb565Buf, rgb888Buf, 0));
    EXPECT_TRUE(SnapShotUtils::RGB565ToRGB888(rgb565Buf, rgb888Buf, RGB565BUF_SIZE));
}

/**
 * @tc.name: WriteRgb888ToJpeg01
 * @tc.desc: write rgb888 to jpeg using invalid data
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, WriteRgb888ToJpeg01, TestSize.Level1)
{
    uint8_t *data = nullptr;
    FILE *file = fopen(defaultFile_.c_str(), "wb");
    ASSERT_NE(nullptr, file);
    EXPECT_FALSE(SnapShotUtils::WriteRgb888ToJpeg(file, 100, 100, data));
    fclose(file);
}

/**
 * @tc.name: WriteRgb888ToJpeg02
 * @tc.desc: write rgb888 to jpeg using invalid file
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, WriteRgb888ToJpeg02, TestSize.Level1)
{
    uint8_t *data = new uint8_t;
    FILE *file = nullptr;
    ASSERT_FALSE(SnapShotUtils::WriteRgb888ToJpeg(file, 100, 100, data));
}

/**
 * @tc.name: Write01
 * @tc.desc: Write default jpeg using valid file names and valid PixelMap
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, Write01, TestSize.Level1)
{
    DisplayId id = DisplayManager::GetInstance().GetDefaultDisplayId();
    std::shared_ptr<Media::PixelMap> pixelMap = DisplayManager::GetInstance().GetScreenshot(id);
    ASSERT_NE(nullptr, pixelMap);
    ASSERT_EQ(true, SnapShotUtils::WriteToJpegWithPixelMap(defaultFile_, *pixelMap));
}

/**
 * @tc.name: Write02
 * @tc.desc: Write default jpeg using valid file names and valid WriteToJpegParam
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, Write02, TestSize.Level1)
{
    DisplayId id = DisplayManager::GetInstance().GetDefaultDisplayId();
    std::shared_ptr<Media::PixelMap> pixelMap = DisplayManager::GetInstance().GetScreenshot(id);
    ASSERT_NE(nullptr, pixelMap);
    WriteToJpegParam param = {
        .width = pixelMap->GetWidth(),
        .height = pixelMap->GetHeight(),
        .stride = pixelMap->GetRowBytes(),
        .format = pixelMap->GetPixelFormat(),
        .data = pixelMap->GetPixels()
    };
    ASSERT_EQ(true, SnapShotUtils::WriteToJpeg(defaultFile_, param));
}

/**
 * @tc.name: Write03
 * @tc.desc: Write custom jpeg using valid file names and valid WriteToJpegParam
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, Write03, TestSize.Level1)
{
    DisplayId id = DisplayManager::GetInstance().GetDefaultDisplayId();
    std::shared_ptr<Media::PixelMap> pixelMap = DisplayManager::GetInstance().GetScreenshot(id);
    ASSERT_NE(nullptr, pixelMap);
    WriteToJpegParam param = {
        .width = (pixelMap->GetWidth() / 2),
        .height = (pixelMap->GetWidth() / 2),
        .stride = pixelMap->GetRowBytes(),
        .format = pixelMap->GetPixelFormat(),
        .data = pixelMap->GetPixels()
    };
    ASSERT_EQ(false, SnapShotUtils::WriteToJpeg(defaultFile_, param));
}

/**
 * @tc.name: Write04
 * @tc.desc: Write pixel map with jpeg, using fd
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, Write04, TestSize.Level1)
{
    DisplayId id = DisplayManager::GetInstance().GetDefaultDisplayId();
    std::shared_ptr<Media::PixelMap> pixelMap = DisplayManager::GetInstance().GetScreenshot(id);
    ASSERT_NE(nullptr, pixelMap);
    int fd = open(defaultFile_.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    ASSERT_NE(-1, fd);
    EXPECT_EQ(true, SnapShotUtils::WriteToJpegWithPixelMap(fd, *pixelMap));
    close(fd);
}

/**
 * @tc.name: Write05
 * @tc.desc: Write custom jpeg using invalid file names and valid WriteToJpegParam
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, Write05, TestSize.Level1)
{
    WriteToJpegParam param = {
        .width = 256,
        .height = 256,
        .stride = 256 * BPP,
        .format = Media::PixelFormat::UNKNOWN,
        .data = new uint8_t
    };
    ASSERT_FALSE(SnapShotUtils::WriteToJpeg("", param));
}

/**
 * @tc.name: Write06
 * @tc.desc: Write custom jpeg using valid file names and invalid WriteToJpegParam
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, Write06, TestSize.Level1)
{
    WriteToJpegParam param = {
        .width = 256,
        .height = 256,
        .stride = 256 * BPP,
        .format = Media::PixelFormat::UNKNOWN,
        .data = nullptr
    };
    ASSERT_FALSE(SnapShotUtils::WriteToJpeg(defaultFile_, param));
}

/**
 * @tc.name: Write07
 * @tc.desc: Write custom jpeg using valid fd and invalid WriteToJpegParam
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, Write07, TestSize.Level1)
{
    WriteToJpegParam param = {
        .width = 256,
        .height = 256,
        .stride = 256 * BPP,
        .format = Media::PixelFormat::UNKNOWN,
        .data = nullptr
    };
    ASSERT_FALSE(SnapShotUtils::WriteToJpeg(1, param));
}

/**
 * @tc.name: Write08
 * @tc.desc: Write custom jpeg using invalid file names and valid WriteToJpegParam
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, Write08, TestSize.Level1)
{
    WriteToJpegParam param = {
        .width = 256,
        .height = 256,
        .stride = 256 * RGB565_PIXEL_BYTES,
        .format = Media::PixelFormat::RGB_565,
        .data = new uint8_t
    };
    ASSERT_FALSE(SnapShotUtils::WriteToJpeg("", param));
}

/**
 * @tc.name: Write09
 * @tc.desc: Write custom jpeg using valid file names and invalid WriteToJpegParam
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, Write09, TestSize.Level1)
{
    WriteToJpegParam param = {
        .width = 256,
        .height = 256,
        .stride = 256 * RGB565_PIXEL_BYTES,
        .format = Media::PixelFormat::RGB_565,
        .data = nullptr
    };
    ASSERT_FALSE(SnapShotUtils::WriteToJpeg(defaultFile_, param));
}

/**
 * @tc.name: Write10
 * @tc.desc: Write custom jpeg using valid fd and invalid WriteToJpegParam
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, Write10, TestSize.Level1)
{
    WriteToJpegParam param = {
        .width = 256,
        .height = 256,
        .stride = 256 * RGB565_PIXEL_BYTES,
        .format = Media::PixelFormat::RGB_565,
        .data = nullptr
    };
    ASSERT_FALSE(SnapShotUtils::WriteToJpeg(1, param));
}

/**
 * @tc.name: CheckWHValid
 * @tc.desc: Check width and height whether valid
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, CheckWHValid, TestSize.Level1)
{
    ASSERT_EQ(false, SnapShotUtils::CheckWHValid(0));
    ASSERT_EQ(true, SnapShotUtils::CheckWHValid(DisplayManager::MAX_RESOLUTION_SIZE_SCREENSHOT));
    ASSERT_EQ(false, SnapShotUtils::CheckWHValid(DisplayManager::MAX_RESOLUTION_SIZE_SCREENSHOT + 1));
}

/**
 * @tc.name: CheckParamValid01
 * @tc.desc: Check jpeg param whether valid width
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, CheckParamValid01, TestSize.Level1)
{
    WriteToJpegParam paramInvalidWidth = {
        .width = DisplayManager::MAX_RESOLUTION_SIZE_SCREENSHOT + 1,
        .height = 0,
        .stride = 0,
        .format = Media::PixelFormat::UNKNOWN,
        .data = nullptr
    };
    ASSERT_EQ(false, SnapShotUtils::CheckParamValid(paramInvalidWidth));
}

/**
 * @tc.name: CheckParamValid02
 * @tc.desc: Check jpeg param whether valid height
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, CheckParamValid02, TestSize.Level1)
{
    WriteToJpegParam paramInvalidHeight = {
        .width = DisplayManager::MAX_RESOLUTION_SIZE_SCREENSHOT,
        .height = 0,
        .stride = 0,
        .format = Media::PixelFormat::UNKNOWN,
        .data = nullptr
    };
    ASSERT_EQ(false, SnapShotUtils::CheckParamValid(paramInvalidHeight));
}

/**
 * @tc.name: CheckParamValid03
 * @tc.desc: Check jpeg param whether valid stride
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, CheckParamValid03, TestSize.Level1)
{
    WriteToJpegParam paramInvalidStride = {
        .width = 256,
        .height = 256,
        .stride = 1,
        .format = Media::PixelFormat::UNKNOWN,
        .data = nullptr
    };
    ASSERT_EQ(false, SnapShotUtils::CheckParamValid(paramInvalidStride));
}

/**
 * @tc.name: CheckParamValid04
 * @tc.desc: Check jpeg param whether valid data
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, CheckParamValid04, TestSize.Level1)
{
    WriteToJpegParam paramInvalidData = {
        .width = 256,
        .height = 256,
        .stride = 256 * BPP,
        .format = Media::PixelFormat::UNKNOWN,
        .data = nullptr
    };
    ASSERT_EQ(false, SnapShotUtils::CheckParamValid(paramInvalidData));
}

/**
 * @tc.name: CheckParamValid05
 * @tc.desc: Check jpeg param whether valid data
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, CheckParamValid05, TestSize.Level1)
{
    WriteToJpegParam paramInvalidData = {
        .width = 256,
        .height = 256,
        .stride = 256 * RGB565_PIXEL_BYTES,
        .format = Media::PixelFormat::RGB_565,
        .data = nullptr
    };
    ASSERT_EQ(false, SnapShotUtils::CheckParamValid(paramInvalidData));
}

/**
 * @tc.name: CheckParamValid06
 * @tc.desc: Check jpeg param whether valid data
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, CheckParamValid06, TestSize.Level1)
{
    WriteToJpegParam paramInvalidData = {
        .width = 256,
        .height = 256,
        .stride = 1,
        .format = Media::PixelFormat::RGB_565,
        .data = nullptr
    };
    ASSERT_EQ(false, SnapShotUtils::CheckParamValid(paramInvalidData));
}

/**
 * @tc.name: CheckParamValid07
 * @tc.desc: Check jpeg param whether valid data
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, CheckParamValid07, TestSize.Level1)
{
    WriteToJpegParam paramInvalidData = {
        .width = 256,
        .height = 256,
        .stride = 256 * RGB888_PIXEL_BYTES,
        .format = Media::PixelFormat::RGB_888,
        .data = nullptr
    };
    ASSERT_EQ(false, SnapShotUtils::CheckParamValid(paramInvalidData));
}

/**
 * @tc.name: CheckParamValid08
 * @tc.desc: Check jpeg param whether valid data
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, CheckParamValid08, TestSize.Level1)
{
    WriteToJpegParam paramInvalidData = {
        .width = 256,
        .height = 256,
        .stride = 1,
        .format = Media::PixelFormat::RGB_888,
        .data = nullptr
    };
    ASSERT_EQ(false, SnapShotUtils::CheckParamValid(paramInvalidData));
}

/**
 * @tc.name: CheckParamValid09
 * @tc.desc: Check jpeg param whether valid width and height
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, CheckParamValid09, TestSize.Level1)
{
    WriteToJpegParam paramInvalidWidthAndHeight = {
        .width = 0,
        .height = 0,
        .stride = 0,
        .format = Media::PixelFormat::RGBA_8888,
        .data = nullptr
    };
    ASSERT_EQ(false, SnapShotUtils::CheckParamValid(paramInvalidWidthAndHeight));
}

/**
 * @tc.name: ProcessDisplayId01
 * @tc.desc: Check RGBA8888ToRGB888
 * @tc.type: FUNC
 */
HWTEST_F(SnapshotUtilsTest, ProcessDisplayId01, TestSize.Level1)
{
    Rosen::DisplayId displayId = 1;
    bool isDisplayIdSet = false;
    ASSERT_EQ(true, SnapShotUtils::ProcessDisplayId(displayId, isDisplayIdSet));
    isDisplayIdSet = true;
    ASSERT_EQ(true, SnapShotUtils::ProcessDisplayId(displayId, isDisplayIdSet));
    displayId = DisplayManager::GetInstance().GetDefaultDisplayId();
    ASSERT_EQ(true, SnapShotUtils::ProcessDisplayId(displayId, isDisplayIdSet));
}
}
} // namespace Rosen
} // namespace OHOS