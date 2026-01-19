/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <hisysevent.h>
#include <parameters.h>
#include "gmock/gmock.h"
#include <functional>
#include "window_manager_hilog.h"

#include "fold_crease_region_controller.h"
#include "screen_session_manager.h"
#include "session/screen/include/screen_session.h"

namespace {
std::string g_logMsg;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
                   const char* msg)
{
    g_logMsg += msg;
}
}
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace DMS {
constexpr uint32_t SLEEP_TIME_US = 100000;

class FoldCreaseRegionControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void FoldCreaseRegionControllerTest::SetUpTestCase() {}

void FoldCreaseRegionControllerTest::TearDownTestCase() {}

void FoldCreaseRegionControllerTest::SetUp() {}

void FoldCreaseRegionControllerTest::TearDown()
{
    LOG_SetCallback(nullptr);
    usleep(SLEEP_TIME_US);
}

namespace {

/**
 * @tc.name: GetCurrentFoldCreaseRegion
 * @tc.desc: test function : GetCurrentFoldCreaseRegion
 * @tc.type: FUNC
 */
HWTEST_F(FoldCreaseRegionControllerTest, GetCurrentFoldCreaseRegion, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto controller = FoldCreaseRegionController();
    controller.isInitModeCreaseRegion_ = false;
    controller.currentFoldCreaseRegion_ = nullptr;
    controller.GetCurrentFoldCreaseRegion();
    EXPECT_TRUE(g_logMsg.find("InitModeCreaseRegion") != std::string::npos);
    g_logMsg.clear();

    controller.isInitModeCreaseRegion_ = true;
    controller.currentFoldCreaseRegion_ = new FoldCreaseRegion(0, {});
    controller.GetCurrentFoldCreaseRegion();
    EXPECT_FALSE(g_logMsg.find("InitModeCreaseRegion") != std::string::npos);
    LOG_SetCallback(nullptr);
}
 
/**
 * @tc.name: GetCreaseRegionRects
 * @tc.desc: test function : GetCreaseRegionRects
 * @tc.type: FUNC
 */
HWTEST_F(FoldCreaseRegionControllerTest, GetCreaseRegionRects, TestSize.Level1)
{
    ScreenId screenId = 6666;
    auto controller = FoldCreaseRegionController();
    Rotation rotation = Rotation::ROTATION_0;
    std::vector<DMRect> rects = controller.GetCreaseRegionRects(screenId, FoldDisplayMode::FULL, rotation);
    EXPECT_TRUE(rects.empty());
    ScreenProperty screenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    ScreenSessionManager::GetInstance().screenSessionMap_.insert({screenId, session});
    std::vector<DMRect> fullRects;
    DMRect fullModeRect = {0, 0, 1000, 2000};
    fullRects.emplace_back(fullModeRect);
    controller.displayModeRects_[FoldDisplayMode::FULL] = fullRects;
    rects = controller.GetCreaseRegionRects(screenId, FoldDisplayMode::FULL, rotation);
    EXPECT_FALSE(rects.empty());
    ScreenSessionManager::GetInstance().screenSessionMap_.erase(screenId);
}

/**
 * @tc.name: GetAllCreaseRegion
 * @tc.desc: test function : GetAllCreaseRegion
 * @tc.type: FUNC
 */
HWTEST_F(FoldCreaseRegionControllerTest, GetAllCreaseRegion, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto controller = FoldCreaseRegionController();
    controller.isInitModeCreaseRegion_ = false;
    controller.currentFoldCreaseRegion_ = nullptr;
    std::vector<FoldCreaseRegionItem> foldCreaseRegionItems;
    controller.GetAllCreaseRegion(foldCreaseRegionItems);
    EXPECT_TRUE(g_logMsg.find("InitModeCreaseRegion") != std::string::npos);
    g_logMsg.clear();

    foldCreaseRegionItems.clear();
    controller.isInitModeCreaseRegion_ = true;
    std::vector<DMRect> fullRects;
    DMRect fullModeRect = {0, 0, 1000, 2000};
    fullRects.emplace_back(fullModeRect);
    controller.displayModeRects_[FoldDisplayMode::FULL] = fullRects;
    ScreenId screenId = 0;
    ScreenProperty screenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    ScreenSessionManager::GetInstance().screenSessionMap_.insert({screenId, session});
    controller.GetAllCreaseRegion(foldCreaseRegionItems);
    EXPECT_EQ(foldCreaseRegionItems.size(), 8);
    ScreenSessionManager::GetInstance().screenSessionMap_.erase(screenId);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetAllCreaseRegionByDisplayMode
 * @tc.desc: test function : GetAllCreaseRegionByDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(FoldCreaseRegionControllerTest, GetAllCreaseRegionByDisplayMode, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto controller = FoldCreaseRegionController();
    std::vector<FoldCreaseRegionItem> foldCreaseRegionItems;
    ScreenId screenId = 6666;
    controller.GetAllCreaseRegionByDisplayMode(FoldDisplayMode::FULL, screenId, foldCreaseRegionItems);
    EXPECT_TRUE(g_logMsg.find("screenSession is null") != std::string::npos);
    g_logMsg.clear();

    ScreenProperty screenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    ScreenSessionManager::GetInstance().screenSessionMap_.insert({screenId, session});
    controller.GetAllCreaseRegionByDisplayMode(FoldDisplayMode::FULL, screenId, foldCreaseRegionItems);
    EXPECT_EQ(foldCreaseRegionItems.size(), 4);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetCreaseRegionByOrientation
 * @tc.desc: test function : GetCreaseRegionByOrientation
 * @tc.type: FUNC
 */
HWTEST_F(FoldCreaseRegionControllerTest, GetCreaseRegionByOrientation, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto controller = FoldCreaseRegionController();
    std::vector<FoldCreaseRegionItem> foldCreaseRegionItems;
    sptr<ScreenSession> session = nullptr;
    controller.GetCreaseRegionByOrientation(session, FoldDisplayMode::FULL,
        DisplayOrientation::PORTRAIT, foldCreaseRegionItems);
    EXPECT_TRUE(g_logMsg.find("screenSession is null") != std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetDisplayModeRectMap
 * @tc.desc: test function : GetDisplayModeRectMap
 * @tc.type: FUNC
 */
HWTEST_F(FoldCreaseRegionControllerTest, GetDisplayModeRectMap, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto controller = FoldCreaseRegionController();
    std::vector<int32_t> foldRect = {0};
    std::vector<DMRect> allRect = controller.ConvertToRectList(foldRect);
    controller.GetDisplayModeRectMap(allRect);
    EXPECT_TRUE(g_logMsg.find("allRect is empty") != std::string::npos);
    g_logMsg.clear();
    foldRect = {0, 0, 100, 200, 0, 0, 1000, 120};
    allRect = controller.ConvertToRectList(foldRect);
    controller.GetDisplayModeRectMap(allRect);
    EXPECT_FALSE(controller.displayModeRects_[FoldDisplayMode::FULL].empty());
    EXPECT_FALSE(controller.displayModeRects_[FoldDisplayMode::COORDINATION].empty());
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RotateSingleRect
 * @tc.desc: test function : RotateSingleRect invalid param
 * @tc.type: FUNC
 */
HWTEST_F(FoldCreaseRegionControllerTest, RotateSingleRectInvalidParam, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DMRect originalRect = {0, 0, 100, 200};
    uint32_t portraitWidth = 0;
    uint32_t portraitHeight = 0;
    Rotation targetRotation = Rotation::ROTATION_0;
    auto controller = FoldCreaseRegionController();
    controller.RotateSingleRect(originalRect, portraitWidth, portraitHeight, targetRotation);
    EXPECT_TRUE(g_logMsg.find("Portrait width or height cannot be zero") != std::string::npos);
    g_logMsg.clear();
    portraitWidth = 100;
    controller.RotateSingleRect(originalRect, portraitWidth, portraitHeight, targetRotation);
    EXPECT_TRUE(g_logMsg.find("Portrait width or height cannot be zero") != std::string::npos);
    originalRect = {0, 0, 0, 0};
    portraitHeight = 200;
    controller.RotateSingleRect(originalRect, portraitWidth, portraitHeight, targetRotation);
    EXPECT_TRUE(g_logMsg.find("Original rect width or height cannot be zero") != std::string::npos);
    g_logMsg.clear();
    originalRect = {0, 0, 100, 0};
    controller.RotateSingleRect(originalRect, portraitWidth, portraitHeight, targetRotation);
    EXPECT_TRUE(g_logMsg.find("Original rect width or height cannot be zero") != std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RotateSingleRect
 * @tc.desc: test function : RotateSingleRect
 * @tc.type: FUNC
 */
HWTEST_F(FoldCreaseRegionControllerTest, RotateSingleRect, TestSize.Level1)
{
    auto controller = FoldCreaseRegionController();
    DMRect originalRect = {100, 0, 10, 200};
    uint32_t portraitWidth = 300;
    uint32_t portraitHeight = 200;
    const int32_t originalPosX = originalRect.posX_;
    const int32_t originalPosY = originalRect.posY_;
    const uint32_t originalWidth = originalRect.width_;
    const uint32_t originalHeight = originalRect.height_;
    Rotation targetRotation = Rotation::ROTATION_0;
    auto ret = controller.RotateSingleRect(originalRect, portraitWidth, portraitHeight, targetRotation);
    EXPECT_EQ(ret.posX_, originalPosX);
    EXPECT_EQ(ret.posY_, originalPosY);
    EXPECT_EQ(ret.width_, originalWidth);
    EXPECT_EQ(ret.height_, originalHeight);

    targetRotation = Rotation::ROTATION_90;
    ret = controller.RotateSingleRect(originalRect, portraitWidth, portraitHeight, targetRotation);
    EXPECT_EQ(ret.posX_, static_cast<int32_t>(portraitHeight - originalPosY - originalHeight));
    EXPECT_EQ(ret.posY_, originalPosX);
    EXPECT_EQ(ret.width_, originalHeight);
    EXPECT_EQ(ret.height_, originalWidth);

    targetRotation = Rotation::ROTATION_180;
    ret = controller.RotateSingleRect(originalRect, portraitWidth, portraitHeight, targetRotation);
    EXPECT_EQ(ret.posX_, static_cast<int32_t>(portraitWidth - originalPosX - originalWidth));
    EXPECT_EQ(ret.posY_, static_cast<int32_t>(portraitHeight - originalPosY - originalHeight));
    EXPECT_EQ(ret.width_, originalWidth);
    EXPECT_EQ(ret.height_, originalHeight);

    targetRotation = Rotation::ROTATION_270;
    ret = controller.RotateSingleRect(originalRect, portraitWidth, portraitHeight, targetRotation);
    EXPECT_EQ(ret.posX_, originalPosY);
    EXPECT_EQ(ret.posY_, static_cast<int32_t>(portraitWidth - originalPosX - originalWidth));
    EXPECT_EQ(ret.width_, originalHeight);
    EXPECT_EQ(ret.height_, originalWidth);
}
}
}
}  // namespace Rosen
}  // namespace OHOS