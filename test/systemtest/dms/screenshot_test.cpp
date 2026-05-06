/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>

#include "common_test_utils.h"
#include "display_test_utils.h"
#include "display_manager_proxy.h"
#include "dm_common.h"
#include "future.h"
#include "pixel_map.h"
#include "screenshot_listener_future.h"
#include "mock_display_manager_adapter.h"
#include "singleton_mocker.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "ScreenshotTest"};
}
using Mocker = SingletonMocker<DisplayManagerAdapter, MockDisplayManagerAdapter>;
using Utils = DisplayTestUtils;
class ScreenshotListener;
class ScreenshotTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    static DisplayId defaultId_;
    static Media::Rect defaultScreen_;
    static Media::Size defaultImage_;
    DisplayId invalidId_ = DISPLAY_ID_INVALID;
    Media::Rect invalidRect_ = {-1, -1, -1, -1};
    uint32_t defaultRot_ = 0;
};

DisplayId ScreenshotTest::defaultId_ = 0;
Media::Rect ScreenshotTest::defaultScreen_ = {0, 0, 0, 0};
Media::Size ScreenshotTest::defaultImage_ = {0, 0};

void ScreenshotTest::SetUpTestCase()
{
    CommonTestUtils::InjectTokenInfoByHapName(0, "com.ohos.systemui", 0);
    const char** perms = new const char *[1];
    perms[0] = "ohos.permission.CAPTURE_SCREEN";
    CommonTestUtils::SetAceessTokenPermission("ScreenshotTest", perms, 1);

    auto display = DisplayManager::GetInstance().GetDisplayById(defaultId_);
    if (display == nullptr) {
        WLOGFE("GetDefaultDisplay: failed!\n");
        return;
    }
    WLOGI("GetDefaultDisplay: id %" PRIu64", w %d, h %d, fps %u\n", display->GetId(), display->GetWidth(),
        display->GetHeight(), display->GetRefreshRate());

    defaultScreen_ = {0, 0, display->GetWidth(), display->GetHeight()};
    defaultImage_ = {display->GetWidth(), display->GetHeight()};
}

void ScreenshotTest::TearDownTestCase()
{
}

void ScreenshotTest::SetUp()
{
}

void ScreenshotTest::TearDown()
{
}

namespace {
/**
 * @tc.name: ScreenShotValid
 * @tc.desc: Check if screenshot of default display's ID is valid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenshotTest, ScreenShotValid01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

    EXPECT_CALL(m->Mock(), GetDefaultDisplayInfo(CONCURRENT_USER_ID_DEFAULT)).Times(1).WillOnce(Return(nullptr));
    EXPECT_EQ(DISPLAY_ID_INVALID, DisplayManager::GetInstance().GetDefaultDisplayId());

    Media::InitializationOptions opt;
    opt.size.width = CommonTestUtils::TEST_IMAGE_WIDTH;
    opt.size.height = CommonTestUtils::TEST_IMAGE_HEIGHT;
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(opt);
    EXPECT_CALL(m->Mock(), GetDisplaySnapshot(_, _, _, _)).Times(1).WillOnce(Return(pixelMap));
    ASSERT_NE(nullptr, DisplayManager::GetInstance().GetScreenshot(0));
}

/**
 * @tc.name: ScreenShotValid
 * @tc.desc: Check if screenshot of invalid display's ID is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenshotTest, ScreenShotValid02, TestSize.Level1)
{
    ASSERT_EQ(nullptr, DisplayManager::GetInstance().GetScreenshot(invalidId_));
}

/**
 * @tc.name: ScreenShotValid
 * @tc.desc: Check if screenshot of default display's ID match default display's Media::Size
 * @tc.type: FUNC
 */
HWTEST_F(ScreenshotTest, ScreenShotValid03, TestSize.Level1)
{
    auto& dm = DisplayManager::GetInstance();
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetDisplayInfo(_)).Times(2);

    Media::InitializationOptions opt;
    opt.size.width = CommonTestUtils::TEST_IMAGE_WIDTH;
    opt.size.height = CommonTestUtils::TEST_IMAGE_HEIGHT;
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(opt);
    EXPECT_CALL(m->Mock(), GetDisplaySnapshot(_, _, _, _)).Times(1).WillOnce(Return(pixelMap));
    std::shared_ptr<Media::PixelMap> screenshot = dm.GetScreenshot(0);
    ASSERT_NE(nullptr, screenshot);

    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    displayInfo->SetWidth(CommonTestUtils::TEST_IMAGE_WIDTH);
    displayInfo->SetHeight(CommonTestUtils::TEST_IMAGE_HEIGHT);
    displayInfo->SetDisplayId(0);
    EXPECT_CALL(m->Mock(), GetDefaultDisplayInfo(CONCURRENT_USER_ID_DEFAULT)).Times(1).WillOnce(Return(displayInfo));

    Media::Size screenSize = {screenshot->GetWidth(), screenshot->GetHeight()};
    ASSERT_TRUE(Utils::SizeEqualToDisplay(dm.GetDefaultDisplay(), screenSize));
}

/**
 * @tc.name: ScreenShotValid
 * @tc.desc: Check if screenshot created by default parameters is valid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenshotTest, ScreenShotValid04, TestSize.Level1)
{
    auto& dm = DisplayManager::GetInstance();
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

    Media::InitializationOptions opt;
    opt.size.width = CommonTestUtils::TEST_IMAGE_WIDTH;
    opt.size.height = CommonTestUtils::TEST_IMAGE_HEIGHT;
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(opt);
    EXPECT_CALL(m->Mock(), GetDisplaySnapshot(_, _, _, _)).Times(1).WillOnce(Return(pixelMap));
    Media::Rect defaultScreen = {0, 0, CommonTestUtils::TEST_IMAGE_WIDTH, CommonTestUtils::TEST_IMAGE_HEIGHT};
    Media::Size defaultImage = {CommonTestUtils::TEST_IMAGE_WIDTH, CommonTestUtils::TEST_IMAGE_HEIGHT};
    std::shared_ptr<Media::PixelMap> screenshot = dm.GetScreenshot(0, defaultScreen, defaultImage, 0);
    ASSERT_NE(nullptr, screenshot);
}

/**
 * @tc.name: ScreenShotValid
 * @tc.desc: Check if screenshot match default imageSize
 * @tc.type: FUNC
 */
HWTEST_F(ScreenshotTest, ScreenShotValid05, TestSize.Level1)
{
    auto& dm = DisplayManager::GetInstance();
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

    EXPECT_CALL(m->Mock(), GetDefaultDisplayInfo(CONCURRENT_USER_ID_DEFAULT)).Times(1).WillOnce(Return(nullptr));
    EXPECT_EQ(DISPLAY_ID_INVALID, DisplayManager::GetInstance().GetDefaultDisplayId());

    Media::InitializationOptions opt;
    opt.size.width = CommonTestUtils::TEST_IMAGE_WIDTH;
    opt.size.height = CommonTestUtils::TEST_IMAGE_HEIGHT;
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(opt);
    EXPECT_CALL(m->Mock(), GetDisplaySnapshot(_, _, _, _)).Times(1).WillOnce(Return(pixelMap));
    Media::Rect defaultScreen = {0, 0, CommonTestUtils::TEST_IMAGE_WIDTH, CommonTestUtils::TEST_IMAGE_HEIGHT};
    Media::Size defaultImage = {CommonTestUtils::TEST_IMAGE_WIDTH, CommonTestUtils::TEST_IMAGE_HEIGHT};
    std::shared_ptr<Media::PixelMap> screenshot = dm.GetScreenshot(0, defaultScreen, defaultImage, 0);
    ASSERT_NE(nullptr, screenshot);

    Media::Size screenSize = {screenshot->GetWidth(), screenshot->GetHeight()};
    ASSERT_TRUE(Utils::SizeEqual(defaultImage, screenSize));

    Media::Size halfDefault = {defaultImage.width / 2, defaultImage.height / 2};
    Media::Rect halfRect = {defaultScreen.left, defaultScreen.top, halfDefault.width, halfDefault.height};
    EXPECT_CALL(m->Mock(), GetDisplaySnapshot(_, _, _, _)).Times(1).WillOnce(Return(pixelMap));
    screenshot = dm.GetScreenshot(0, halfRect, defaultImage, 0);
    ASSERT_NE(nullptr, screenshot);
    screenSize = {screenshot->GetWidth(), screenshot->GetHeight()};
    ASSERT_TRUE(Utils::SizeEqual(defaultImage, screenSize));

    Media::Size halfSize = {halfDefault.width, halfDefault.height};
    EXPECT_CALL(m->Mock(), GetDisplaySnapshot(_, _, _, _)).Times(1).WillOnce(Return(pixelMap));
    screenshot = dm.GetScreenshot(0, defaultScreen, halfSize, 0);
    ASSERT_NE(nullptr, screenshot);
    screenSize = {screenshot->GetWidth(), screenshot->GetHeight()};
    ASSERT_TRUE(Utils::SizeEqual(halfSize, screenSize));
}

/**
 * @tc.name: ScreenShotValid
 * @tc.desc: Check if screenshot created by default parameters match default display's Media::Size
 * @tc.type: FUNC
 */
HWTEST_F(ScreenshotTest, ScreenShotValid06, TestSize.Level1)
{
    auto& dm = DisplayManager::GetInstance();
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetDisplayInfo(_)).Times(2);
    
    Media::InitializationOptions opt;
    opt.size.width = CommonTestUtils::TEST_IMAGE_WIDTH;
    opt.size.height = CommonTestUtils::TEST_IMAGE_HEIGHT;
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(opt);
    EXPECT_CALL(m->Mock(), GetDisplaySnapshot(_, _, _, _)).Times(1).WillOnce(Return(pixelMap));
    Media::Rect defaultScreen = {0, 0, CommonTestUtils::TEST_IMAGE_WIDTH, CommonTestUtils::TEST_IMAGE_HEIGHT};
    Media::Size defaultImage = {CommonTestUtils::TEST_IMAGE_WIDTH, CommonTestUtils::TEST_IMAGE_HEIGHT};
    std::shared_ptr<Media::PixelMap> screenshot = dm.GetScreenshot(0, defaultScreen, defaultImage, 0);
    ASSERT_NE(nullptr, screenshot);

    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    displayInfo->SetWidth(CommonTestUtils::TEST_IMAGE_WIDTH);
    displayInfo->SetHeight(CommonTestUtils::TEST_IMAGE_HEIGHT);
    displayInfo->SetDisplayId(0);
    EXPECT_CALL(m->Mock(), GetDefaultDisplayInfo(CONCURRENT_USER_ID_DEFAULT)).Times(1).WillOnce(Return(displayInfo));
    Media::Size screenSize = {screenshot->GetWidth(), screenshot->GetHeight()};
    ASSERT_TRUE(Utils::SizeEqualToDisplay(dm.GetDefaultDisplay(), screenSize));
}

/**
 * @tc.name: ScreenShotValid
 * @tc.desc: Check if screenshot created by invalid display ID is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenshotTest, ScreenShot07, TestSize.Level1)
{
    auto& dm = DisplayManager::GetInstance();
    std::shared_ptr<Media::PixelMap> screenshot = dm.GetScreenshot(invalidId_, defaultScreen_,
                                                                   defaultImage_, defaultRot_);
    ASSERT_EQ(nullptr, screenshot);
}

/**
 * @tc.name: ScreenShotValid
 * @tc.desc: Check if screenshot created by invalid screenRect is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenshotTest, ScreenShot08, TestSize.Level1)
{
    auto& dm = DisplayManager::GetInstance();
    std::shared_ptr<Media::PixelMap> screenshot = dm.GetScreenshot(defaultId_, invalidRect_,
                                                                   defaultImage_, defaultRot_);
    ASSERT_EQ(nullptr, screenshot);
    Media::Rect invalidScreen = {invalidRect_.left, defaultScreen_.top, defaultScreen_.width, defaultScreen_.height};
    screenshot = dm.GetScreenshot(defaultId_, invalidScreen, defaultImage_, defaultRot_);
    ASSERT_EQ(nullptr, screenshot);
    invalidScreen = {defaultScreen_.left, defaultScreen_.top, invalidRect_.width, defaultScreen_.height};
    screenshot = dm.GetScreenshot(defaultId_, invalidScreen, defaultImage_, defaultRot_);
    ASSERT_EQ(nullptr, screenshot);
}

/**
 * @tc.name: ScreenShotValid
 * @tc.desc: Check if screenshot created by invalid imageSize is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenshotTest, ScreenShot09, TestSize.Level1)
{
    auto& dm = DisplayManager::GetInstance();
    Media::Size invalidSize = {invalidRect_.width, invalidRect_.height};
    std::shared_ptr<Media::PixelMap> screenshot = dm.GetScreenshot(defaultId_, defaultScreen_,
                                                                   invalidSize, defaultRot_);
    ASSERT_EQ(nullptr, screenshot);
    invalidSize = {invalidRect_.width, defaultScreen_.height};
    screenshot = dm.GetScreenshot(defaultId_, defaultScreen_, invalidSize, defaultRot_);
    ASSERT_EQ(nullptr, screenshot);
    invalidSize = {defaultScreen_.width, invalidRect_.height};
    screenshot = dm.GetScreenshot(defaultId_, defaultScreen_, invalidSize, defaultRot_);
    ASSERT_EQ(nullptr, screenshot);
}

/**
 * @tc.name: ScreenShotListener01
 * @tc.desc: Check if screenshot listener info valid
 * @tc.type: FUNC
 * @tc.require: issueI5G62O
 */
HWTEST_F(ScreenshotTest, ScreenShotListener01, TestSize.Level1)
{
    auto& dm = DisplayManager::GetInstance();
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

    Media::InitializationOptions opt;
    opt.size.width = CommonTestUtils::TEST_IMAGE_WIDTH;
    opt.size.height = CommonTestUtils::TEST_IMAGE_HEIGHT;
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(opt);
    EXPECT_CALL(m->Mock(), GetDisplaySnapshot(_, _, _, _)).Times(1).WillOnce(Return(pixelMap));
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    EXPECT_CALL(m->Mock(), UnregisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));

    sptr<ScreenshotListenerFuture> screenShotListener = new ScreenshotListenerFuture();
    dm.RegisterScreenshotListener(screenShotListener);
    std::shared_ptr<Media::PixelMap> screenshot = dm.GetScreenshot(defaultId_);
    auto info = screenShotListener->future_.GetResult(1000);

    ASSERT_NE(nullptr, screenshot);

    dm.UnregisterScreenshotListener(screenShotListener);
}
} // namespace
} // namespace Rosen
} // namespace OHOS